#include "YBaseLib/Log.h"
#include "YBaseLib/NumericLimits.h"
#include "YBaseLib/PODArray.h"
#include "YRenderLib/Vulkan/VulkanCommon.h"
#include "YRenderLib/Vulkan/VulkanDefines.h"
#include "YRenderLib/Vulkan/VulkanGPUContext.h"
#include "YRenderLib/Vulkan/VulkanImports.h"
#include <SDL/SDL_syswm.h>
Log_SetChannel(VulkanBackend);

static bool CreateVulkanInstance(VkInstance* pInstance)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "YRenderLib";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "YRenderLib";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledExtensionCount = 0;
    instanceCreateInfo.ppEnabledExtensionNames = nullptr;
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;

    VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, pInstance);
    if (res != VK_SUCCESS)
    {
        LOG_VULKAN_ERROR(res, "vkCreateInstance failed: ");
        return false;
    }

    return true;
}

static bool SelectPhysicalDevice(VkInstance instance, VkPhysicalDevice* pPhysicalDevice)
{
    uint32 physicalDeviceCount;
    VkResult res = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (res != VK_SUCCESS)
    {
        LOG_VULKAN_ERROR(res, "vkEnumeratePhysicalDevices failed: ");
        return false;
    }

    Log_InfoPrintf("%u vulkan physical devices available", physicalDeviceCount);
    if (physicalDeviceCount == 0)
        return false;

    VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*)alloca(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    res = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);
    if (res != VK_SUCCESS)
    {
        LOG_VULKAN_ERROR(res, "vkEnumeratePhysicalDevices failed: ");
        return false;
    }

    for (uint32 i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);
        Log_InfoPrintf("  %u: %s", i, properties.deviceName);
    }

    *pPhysicalDevice = physicalDevices[0];
    return true;
}

static bool CreateSurface(VkInstance instance, SDL_Window* pSDLWindow, VkSurfaceKHR* pSurface)
{
#if defined(Y_PLATFORM_WINDOWS)
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(pSDLWindow, &info))
    {
        Log_ErrorPrintf("SDL_GetWindowWMInfo failed: %s", SDL_GetError());
        return false;
    }

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hinstance = nullptr;
    surfaceCreateInfo.hwnd = info.info.win.window;

    VkResult res = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, pSurface);
    if (res != VK_SUCCESS)
    {
        LOG_VULKAN_ERROR(res, "vkCreateWin32SurfaceKHR failed: ");
        return false;
    }

    return true;
    
#else
    return false;
#endif
}

static bool SelectExtensions(VkInstance instance, VkPhysicalDevice physicalDevice, PODArray<const char*>& extensionNameList)
{
    uint32 extensionCount;
    VkResult res = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    if (res != VK_SUCCESS)
    {
        LOG_VULKAN_ERROR(res, "vkEnumerateDeviceExtensionProperties failed: ");
        return false;
    }

    if (extensionCount == 0)
    {
        Log_ErrorPrintf("No extensions supported by device.");
        return false;
    }

    VkExtensionProperties* availableExtensionsList = (VkExtensionProperties*)alloca(sizeof(VkExtensionProperties) * extensionCount);
    res = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensionsList);
    if (res != VK_SUCCESS)
    {
        LOG_VULKAN_ERROR(res, "vkEnumerateDeviceExtensionProperties failed: ");
        return false;
    }

    SmallString extensionsString;
    for (uint32 i = 0; i < extensionCount; i++)
        extensionsString.AppendFormattedString("%s%s", (i > 0) ? ", " : "Available extensions: ", availableExtensionsList[i].extensionName);
    Log_DevPrint(extensionsString.GetCharArray());

    auto checkForExtension = [&extensionNameList, availableExtensionsList, extensionCount](const char* name, bool required) -> bool
    {
        for (uint32 i = 0; i < extensionCount; i++)
        {
            if (Y_strcmp(availableExtensionsList[i].extensionName, name) == 0)
            {
                extensionNameList.Add(availableExtensionsList[i].extensionName);
                return true;
            }
        }

        if (required)
        {
            Log_ErrorPrintf("Missing required extension %s.", name);
            return false;
        }

        return true;
    };

    if (!checkForExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true))
    {
        return false;
    }

    return true;
}

static bool CreateDevice(VkInstance instance, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice* pDevice)
{
    // TODO: Cache queue types/indices
    uint32 queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0)
    {
        Log_ErrorPrintf("No queue families found.");
        return false;
    }
    
    VkQueueFamilyProperties* queueFamilyProperties = (VkQueueFamilyProperties*)alloca(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties);
    Log_InfoPrintf("%u vulkan queue families", queueFamilyCount);
    
    // Find a graphics queue
    uint32 graphicsQueueFamilyIndex = Y_UINT32_MAX;
    for (uint32 i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            // Check that it can present to our surface from this queue
            VkBool32 presentSupported;
            VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupported);
            if (res != VK_SUCCESS)
            {
                LOG_VULKAN_ERROR(res, "vkGetPhysicalDeviceSurfaceSupportKHR failed: ");
                return false;
            }

            if (presentSupported)
            {
                graphicsQueueFamilyIndex = i;
                break;
            }
        }
    }
    if (graphicsQueueFamilyIndex == Y_UINT32_MAX)
    {
        Log_ErrorPrintf("Failed to find an acceptable graphics queue.");
        return false;
    }

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    static constexpr float queuePriorities[] = { 1.0f };
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;

    PODArray<const char*> enabledExtensionNames;
    if (!SelectExtensions(instance, physicalDevice, enabledExtensionNames))
        return false;

    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = enabledExtensionNames.GetSize();
    createInfo.ppEnabledExtensionNames = enabledExtensionNames.GetBasePointer();
    createInfo.pEnabledFeatures = nullptr;

    VkResult res = vkCreateDevice(physicalDevice, &createInfo, nullptr, pDevice);
    if (res != VK_SUCCESS)
    {
        LOG_VULKAN_ERROR(res, "vkCreateDevice failed: ");
        return false;
    }

    return true;
}

bool VulkanBackend_Create(const RendererInitializationParameters* pCreateParameters, SDL_Window* pSDLWindow, GPUDevice** ppDevice, GPUContext** ppContext, GPUOutputBuffer** ppOutputBuffer)
{
    if (!LoadVulkanLibrary())
        return false;

    VkInstance instance;
    if (!CreateVulkanInstance(&instance))
    {
        UnloadVulkanLibrary();
        return false;
    }

    VkPhysicalDevice physicalDevice;
    if (!SelectPhysicalDevice(instance, &physicalDevice))
    {
        vkDestroyInstance(instance, nullptr);
        UnloadVulkanLibrary();
        return false;
    }

    VkSurfaceKHR surface;
    if (!CreateSurface(instance, pSDLWindow, &surface))
    {
        vkDestroyInstance(instance, nullptr);
        UnloadVulkanLibrary();
        return false;
    }

    VkDevice device;
    if (!CreateDevice(instance, physicalDevice, surface, &device))
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        UnloadVulkanLibrary();
        return false;
    }

    return false;
}
