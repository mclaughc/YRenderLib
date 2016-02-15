#include "YBaseLib/Log.h"
#include "YRenderLib/D3D11/D3D11Defines.h"
#include "YRenderLib/D3D11/D3D11GPUContext.h"
#include "YRenderLib/D3D11/D3D11GPUDevice.h"
#include "YRenderLib/D3D11/D3D11GPUInputLayout.h"
#include <wrl/client.h>
Log_SetChannel(D3D11GPUContext);

#pragma comment(lib, "d3dcompiler.lib")

D3D11GPUInputLayout::D3D11GPUInputLayout(GPU_VERTEX_ELEMENT_DESC* pElements, uint32 nElements, ID3D11InputLayout* pD3DInputLayout)
    : GPUInputLayout(pElements, nElements)
    , m_pD3DInputLayout(pD3DInputLayout)
{

}

D3D11GPUInputLayout::~D3D11GPUInputLayout()
{
    m_pD3DInputLayout->Release();
}

void D3D11GPUInputLayout::GetMemoryUsage(uint32 *cpuMemoryUsage, uint32 *gpuMemoryUsage) const
{
    if (cpuMemoryUsage)
        *cpuMemoryUsage = sizeof(this) + sizeof(GPU_VERTEX_ELEMENT_DESC) * m_nElements;
    if (gpuMemoryUsage)
        *gpuMemoryUsage = 16 * m_nElements;
}

void D3D11GPUInputLayout::SetDebugName(const char *name)
{
    D3D11Helpers::SetD3D11DeviceChildDebugName(m_pD3DInputLayout, name);
}

GPUInputLayout* D3D11GPUDevice::CreateInputLayout(const GPU_VERTEX_ELEMENT_DESC* pElements, uint32 nElements)
{
    HRESULT hResult;

    // convert the vertex elements to d3d format
    D3D11_INPUT_ELEMENT_DESC *pD3DInputElements = (nElements > 0) ? (D3D11_INPUT_ELEMENT_DESC *)alloca(sizeof(D3D11_INPUT_ELEMENT_DESC)* nElements) : nullptr;
    for (uint32 i = 0; i < nElements; i++)
    {
        const GPU_VERTEX_ELEMENT_DESC &elementDesc = pElements[i];
        pD3DInputElements[i].SemanticName = D3D11TypeConversion::VertexElementSemanticToString(elementDesc.Semantic);
        pD3DInputElements[i].SemanticIndex = elementDesc.SemanticIndex;
        pD3DInputElements[i].Format = D3D11TypeConversion::VertexElementTypeToDXGIFormat(elementDesc.Type);
        pD3DInputElements[i].InputSlot = elementDesc.StreamIndex;
        pD3DInputElements[i].AlignedByteOffset = elementDesc.StreamOffset;
        pD3DInputElements[i].InputSlotClass = (elementDesc.InstanceStepRate != 0) ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
        pD3DInputElements[i].InstanceDataStepRate = elementDesc.InstanceStepRate;
    }

    // create shader code that doesn't return anything, but contains the input signature
    String vertexShaderCode;
    vertexShaderCode.Reserve(200 + 80 * nElements);
    vertexShaderCode.AppendString("void main(");
    for (uint32 i = 0; i < nElements; i++)
    {
        const GPU_VERTEX_ELEMENT_DESC &elementDesc = pElements[i];
        DebugAssert(elementDesc.Semantic < GPU_VERTEX_ELEMENT_SEMANTIC_COUNT);
        DebugAssert(elementDesc.Type < GPU_VERTEX_ELEMENT_TYPE_COUNT);
        vertexShaderCode.AppendFormattedString("in %s inputElement%u_stream%u_offset%u : %s%u,\n          ",
            D3D11TypeConversion::VertexElementTypeToShaderTypeString(elementDesc.Type),
            i,
            elementDesc.StreamIndex,
            elementDesc.StreamOffset,
            D3D11TypeConversion::VertexElementSemanticToString(elementDesc.Semantic),
            elementDesc.SemanticIndex);
    }
    vertexShaderCode.AppendString("out float4 out_position : SV_Position)\n");
    vertexShaderCode.AppendString("{\n");
    vertexShaderCode.AppendString("    out_position = float4(0, 0, 0, 0);\n");
    vertexShaderCode.AppendString("}\n");

    Microsoft::WRL::ComPtr<ID3DBlob> pVertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pErrorMessageBlob;
    hResult = D3DCompile(vertexShaderCode.GetCharArray(), vertexShaderCode.GetLength(), "", nullptr, nullptr, "main", "vs_4_0", 0, 0, &pVertexShaderBlob, &pErrorMessageBlob);
    if (FAILED(hResult))
    {
        Log_ErrorPrintf("D3DCompile failed with hResult %08X", hResult);
        if (pErrorMessageBlob)
            Log_ErrorPrint((const char *)pErrorMessageBlob->GetBufferPointer());

        return nullptr;
    }

    // create input layout object
    ID3D11InputLayout* pD3DInputLayout;
    hResult = m_pD3DDevice->CreateInputLayout(pD3DInputElements, nElements, pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), &pD3DInputLayout);
    if (FAILED(hResult))
    {
        Log_ErrorPrintf("CreateInputLayout failed with hResult %08X", hResult);
        return nullptr;
    }

    // create wrapper object
    GPU_VERTEX_ELEMENT_DESC* pElementsCopy = (nElements > 0) ? new GPU_VERTEX_ELEMENT_DESC[nElements] : nullptr;
    if (nElements > 0)
        Y_memcpy(pElementsCopy, pElements, sizeof(GPU_VERTEX_ELEMENT_DESC) * nElements);

    return new D3D11GPUInputLayout(pElementsCopy, nElements, pD3DInputLayout);
}

