#include "YBaseLib/Log.h"
#include "YRenderLib/D3D11/D3D11Defines.h"
#include "YRenderLib/D3D11/D3D11GPUBuffer.h"
#include "YRenderLib/D3D11/D3D11GPUContext.h"
#include "YRenderLib/D3D11/D3D11GPUDevice.h"
#include "YRenderLib/D3D11/D3D11GPUQuery.h"
#include "YRenderLib/D3D11/D3D11GPUOutputBuffer.h"
#include "YRenderLib/D3D11/D3D11GPUShaderProgram.h"
#include "YRenderLib/D3D11/D3D11GPUTexture.h"
#include "YRenderLib/Util.h"
#include <wrl/client.h>
Log_SetChannel(D3D11GPUContext);

// d3d11 libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "uxtheme.lib")

bool D3D11RenderBackend_Create(const RendererInitializationParameters *pCreateParameters, SDL_Window *pSDLWindow, GPUDevice **ppDevice, GPUContext **ppContext, GPUOutputBuffer **ppOutputBuffer)
{
    HRESULT hResult;

    // select formats
    DXGI_FORMAT swapChainBackBufferFormat = D3D11TypeConversion::PixelFormatToDXGIFormat(pCreateParameters->BackBufferFormat);
    DXGI_FORMAT swapChainDepthStencilBufferFormat = (pCreateParameters->DepthStencilBufferFormat != PIXEL_FORMAT_UNKNOWN) ? D3D11TypeConversion::PixelFormatToDXGIFormat(pCreateParameters->DepthStencilBufferFormat) : DXGI_FORMAT_UNKNOWN;
    if (swapChainBackBufferFormat == DXGI_FORMAT_UNKNOWN || (pCreateParameters->DepthStencilBufferFormat != PIXEL_FORMAT_UNKNOWN && swapChainDepthStencilBufferFormat == DXGI_FORMAT_UNKNOWN))
    {
        Log_ErrorPrintf("D3D11RenderBackend::Create: Invalid swap chain format (%s / %s)", NameTable_GetNameString(NameTables::PixelFormat, pCreateParameters->BackBufferFormat), NameTable_GetNameString(NameTables::PixelFormat, pCreateParameters->DepthStencilBufferFormat));
        return false;
    }

    // determine driver type
    D3D_DRIVER_TYPE driverType;
    if (pCreateParameters->D3DForceWarpDevice)
        driverType = D3D_DRIVER_TYPE_WARP;
    else
        driverType = D3D_DRIVER_TYPE_HARDWARE;

    // feature levels
    D3D_FEATURE_LEVEL acquiredFeatureLevel;
    static const D3D_FEATURE_LEVEL requestedFeatureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    // device flags
    UINT deviceFlags = 0;
    if (pCreateParameters->CreateDebugDevice)
    {
        Log_PerfPrintf("Creating a debug Direct3D 11 device, performance will suffer as a result.");
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }

    // create the device
    Microsoft::WRL::ComPtr<ID3D11Device> pD3DDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> pD3DImmediateContext;
    hResult = D3D11CreateDevice(nullptr, driverType, nullptr, deviceFlags,
                                requestedFeatureLevels, countof(requestedFeatureLevels),
                                D3D11_SDK_VERSION, &pD3DDevice, &acquiredFeatureLevel,
                                &pD3DImmediateContext);

    if (FAILED(hResult))
    {
        Log_ErrorPrintf("D3D11RenderBackend::Create: Could not create D3D11 device: %08X.", hResult);
        return false;
    }

    // logging
    Log_DevPrintf("D3D11RenderBackend::Create: Returned a device with feature level %s.", D3D11TypeConversion::D3DFeatureLevelToString(acquiredFeatureLevel));

    // test feature levels
    RENDERER_FEATURE_LEVEL featureLevel;
    TEXTURE_PLATFORM texturePlatform;
    SHADER_PROGRAM_BYTECODE_TYPE shaderProgramType;
    if (acquiredFeatureLevel == D3D_FEATURE_LEVEL_10_0)
    {
        featureLevel = RENDERER_FEATURE_LEVEL_SM4;
        texturePlatform = TEXTURE_PLATFORM_DXTC;
        shaderProgramType = SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM40;
    }
    else if (acquiredFeatureLevel == D3D_FEATURE_LEVEL_10_1)
    {
        featureLevel = RENDERER_FEATURE_LEVEL_SM4;
        texturePlatform = TEXTURE_PLATFORM_DXTC;
        shaderProgramType = SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM41;
    }
    else if (acquiredFeatureLevel == D3D_FEATURE_LEVEL_11_0)
    {
        featureLevel = RENDERER_FEATURE_LEVEL_SM5;
        texturePlatform = TEXTURE_PLATFORM_DXTC;
        shaderProgramType = SHADER_PROGRAM_BYTECODE_TYPE_D3D_SM50;
    }
    else
    {
        Log_ErrorPrintf("D3D11RenderBackend::Create: Returned a device with an unusable feature level: %s (%08X)", D3D11TypeConversion::D3DFeatureLevelToString(acquiredFeatureLevel), acquiredFeatureLevel);
        return false;
    }

    // retrieve handles to DXGI from the created device
    Microsoft::WRL::ComPtr<IDXGIFactory> pDXGIFactory;
    Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
    {
        // get a temporary handle to the dxgi device interface
        Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice;
        hResult = pD3DDevice.As(&pDXGIDevice);
        if (FAILED(hResult))
        {
            Log_ErrorPrintf("D3D11RenderBackend::Create: Could not get DXGI device from D3D11 device.", hResult);
            return false;
        }

        // get the adapter from this
        hResult = pDXGIDevice->GetAdapter(&pDXGIAdapter);
        if (FAILED(hResult))
        {
            Log_ErrorPrintf("D3D11RenderBackend::Create: Could not get DXGI adapter from device.", hResult);
            return false;
        }

        // get the parent of the adapter (factory)
        hResult = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), &pDXGIFactory);
        if (FAILED(hResult))
        {
            Log_ErrorPrintf("D3D11RenderBackend::Create: Could not get DXGI factory from device.", hResult);
            return false;
        }
    }

    // print device name
    {
        // get adapter desc
        DXGI_ADAPTER_DESC DXGIAdapterDesc;
        hResult = pDXGIAdapter->GetDesc(&DXGIAdapterDesc);
        DebugAssert(hResult == S_OK);

        char deviceName[128];
        WideCharToMultiByte(CP_ACP, 0, DXGIAdapterDesc.Description, -1, deviceName, countof(deviceName), NULL, NULL);
        Log_InfoPrintf("D3D11RenderBackend using DXGI Adapter: %s.", deviceName);
        Log_InfoPrintf("Texture Platform: %s", NameTable_GetNameString(NameTables::TexturePlatform, texturePlatform));
    }

    // check for threading support
    D3D11_FEATURE_DATA_THREADING threadingFeatureData;
    if (FAILED(hResult = pD3DDevice->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingFeatureData, sizeof(threadingFeatureData))))
    {
        Log_ErrorPrintf("D3D11RenderBackend::Create: CheckFeatureSupport(D3D11_FEATURE_THREADING) failed with hResult %08X", hResult);
        return false;
    }

    // cap warnings
    if (threadingFeatureData.DriverConcurrentCreates != TRUE)
        Log_WarningPrint("Direct3D device driver does not support concurrent resource creation. This may cause some stuttering during streaming.");

    // get the 11.1 device
    Microsoft::WRL::ComPtr<ID3D11Device1> pD3DDevice1;
    hResult = pD3DDevice.As(&pD3DDevice1);
    if (FAILED(hResult))
        Log_WarningPrintf("Failed to retrieve ID3D11Device1 interface with hResult %08X. 11.1 features will not be used.", hResult);
    else
        Log_InfoPrintf("Using Direct3D 11.1 features.");

    // create device wrapper class
    D3D11GPUDevice *pGPUDevice = new D3D11GPUDevice(pDXGIFactory.Get(), pDXGIAdapter.Get(), pD3DDevice.Get(), pD3DDevice1.Get(), acquiredFeatureLevel, featureLevel, texturePlatform, shaderProgramType, swapChainBackBufferFormat, swapChainDepthStencilBufferFormat);

    // create context wrapper class
    D3D11GPUContext *pGPUContext = new D3D11GPUContext(pGPUDevice, pD3DDevice.Get(), pD3DDevice1.Get(), pD3DImmediateContext.Get());
    if (!pGPUDevice->Create() || !pGPUContext->Create())
    {
        pGPUContext->Release();
        pGPUDevice->Release();
        return false;
    }

    // create implicit swap chain
    GPUOutputBuffer *pOutputBuffer = nullptr;
    if (pSDLWindow != nullptr)
    {
        // pass through to normal method
        pOutputBuffer = pGPUDevice->CreateOutputBuffer(pSDLWindow, pCreateParameters->ImplicitSwapChainVSyncType);
        if (pOutputBuffer == nullptr)
        {
            pGPUContext->Release();
            pGPUDevice->Release();
            return false;
        }

        // bind to context
        pGPUContext->SetOutputBuffer(pOutputBuffer);
    }

    // set pointers
    *ppDevice = pGPUDevice;
    *ppContext = pGPUContext;
    *ppOutputBuffer = pOutputBuffer;

    Log_InfoPrint("D3D11 render backend creation successful.");
    return true;
}
