#include "YBaseLib/Atomic.h"
#include "YBaseLib/Log.h"
#include "YRenderLib/Vulkan/VulkanImports.h"
Log_SetChannel(VulkanImports);

PFN_vkCreateInstance vkCreateInstance;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueueWaitIdle vkQueueWaitIdle;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkMapMemory vkMapMemory;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
PFN_vkQueueBindSparse vkQueueBindSparse;
PFN_vkCreateFence vkCreateFence;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkResetFences vkResetFences;
PFN_vkGetFenceStatus vkGetFenceStatus;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkCreateEvent vkCreateEvent;
PFN_vkDestroyEvent vkDestroyEvent;
PFN_vkGetEventStatus vkGetEventStatus;
PFN_vkSetEvent vkSetEvent;
PFN_vkResetEvent vkResetEvent;
PFN_vkCreateQueryPool vkCreateQueryPool;
PFN_vkDestroyQueryPool vkDestroyQueryPool;
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkCreateBufferView vkCreateBufferView;
PFN_vkDestroyBufferView vkDestroyBufferView;
PFN_vkCreateImage vkCreateImage;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkCreatePipelineCache vkCreatePipelineCache;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
PFN_vkMergePipelineCaches vkMergePipelineCaches;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCreateComputePipelines vkCreateComputePipelines;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkResetDescriptorPool vkResetDescriptorPool;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkDestroyRenderPass vkDestroyRenderPass;
PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkResetCommandPool vkResetCommandPool;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkResetCommandBuffer vkResetCommandBuffer;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
PFN_vkCmdDispatch vkCmdDispatch;
PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCmdCopyImage vkCmdCopyImage;
PFN_vkCmdBlitImage vkCmdBlitImage;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
PFN_vkCmdFillBuffer vkCmdFillBuffer;
PFN_vkCmdClearColorImage vkCmdClearColorImage;
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
PFN_vkCmdClearAttachments vkCmdClearAttachments;
PFN_vkCmdResolveImage vkCmdResolveImage;
PFN_vkCmdSetEvent vkCmdSetEvent;
PFN_vkCmdResetEvent vkCmdResetEvent;
PFN_vkCmdWaitEvents vkCmdWaitEvents;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCmdBeginQuery vkCmdBeginQuery;
PFN_vkCmdEndQuery vkCmdEndQuery;
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
PFN_vkCmdPushConstants vkCmdPushConstants;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdNextSubpass vkCmdNextSubpass;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;

PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

#if defined(Y_PLATFORM_WINDOWS)

PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;

#endif

struct FunctionTableEntry
{
    const char* FunctionName;
    void** FunctionPointerAddress;
    bool Required;
};

#define FUNCTION_TABLE_ENTRY(name, required) { #name, reinterpret_cast<void**>(&name), required }

static const FunctionTableEntry s_functionTableEntries[] =
{
    FUNCTION_TABLE_ENTRY(vkCreateInstance, true),
    FUNCTION_TABLE_ENTRY(vkDestroyInstance, true),
    FUNCTION_TABLE_ENTRY(vkEnumeratePhysicalDevices, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceFeatures, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceFormatProperties, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceImageFormatProperties, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceProperties, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceQueueFamilyProperties, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceMemoryProperties, true),
    FUNCTION_TABLE_ENTRY(vkGetInstanceProcAddr, true),
    FUNCTION_TABLE_ENTRY(vkGetDeviceProcAddr, true),
    FUNCTION_TABLE_ENTRY(vkCreateDevice, true),
    FUNCTION_TABLE_ENTRY(vkDestroyDevice, true),
    FUNCTION_TABLE_ENTRY(vkEnumerateInstanceExtensionProperties, true),
    FUNCTION_TABLE_ENTRY(vkEnumerateDeviceExtensionProperties, true),
    FUNCTION_TABLE_ENTRY(vkEnumerateInstanceLayerProperties, true),
    FUNCTION_TABLE_ENTRY(vkEnumerateDeviceLayerProperties, true),
    FUNCTION_TABLE_ENTRY(vkGetDeviceQueue, true),
    FUNCTION_TABLE_ENTRY(vkQueueSubmit, true),
    FUNCTION_TABLE_ENTRY(vkQueueWaitIdle, true),
    FUNCTION_TABLE_ENTRY(vkDeviceWaitIdle, true),
    FUNCTION_TABLE_ENTRY(vkAllocateMemory, true),
    FUNCTION_TABLE_ENTRY(vkFreeMemory, true),
    FUNCTION_TABLE_ENTRY(vkMapMemory, true),
    FUNCTION_TABLE_ENTRY(vkUnmapMemory, true),
    FUNCTION_TABLE_ENTRY(vkFlushMappedMemoryRanges, true),
    FUNCTION_TABLE_ENTRY(vkInvalidateMappedMemoryRanges, true),
    FUNCTION_TABLE_ENTRY(vkGetDeviceMemoryCommitment, true),
    FUNCTION_TABLE_ENTRY(vkBindBufferMemory, true),
    FUNCTION_TABLE_ENTRY(vkBindImageMemory, true),
    FUNCTION_TABLE_ENTRY(vkGetBufferMemoryRequirements, true),
    FUNCTION_TABLE_ENTRY(vkGetImageMemoryRequirements, true),
    FUNCTION_TABLE_ENTRY(vkGetImageSparseMemoryRequirements, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceSparseImageFormatProperties, true),
    FUNCTION_TABLE_ENTRY(vkQueueBindSparse, true),
    FUNCTION_TABLE_ENTRY(vkCreateFence, true),
    FUNCTION_TABLE_ENTRY(vkDestroyFence, true),
    FUNCTION_TABLE_ENTRY(vkResetFences, true),
    FUNCTION_TABLE_ENTRY(vkGetFenceStatus, true),
    FUNCTION_TABLE_ENTRY(vkWaitForFences, true),
    FUNCTION_TABLE_ENTRY(vkCreateSemaphore, true),
    FUNCTION_TABLE_ENTRY(vkDestroySemaphore, true),
    FUNCTION_TABLE_ENTRY(vkCreateEvent, true),
    FUNCTION_TABLE_ENTRY(vkDestroyEvent, true),
    FUNCTION_TABLE_ENTRY(vkGetEventStatus, true),
    FUNCTION_TABLE_ENTRY(vkSetEvent, true),
    FUNCTION_TABLE_ENTRY(vkResetEvent, true),
    FUNCTION_TABLE_ENTRY(vkCreateQueryPool, true),
    FUNCTION_TABLE_ENTRY(vkDestroyQueryPool, true),
    FUNCTION_TABLE_ENTRY(vkGetQueryPoolResults, true),
    FUNCTION_TABLE_ENTRY(vkCreateBuffer, true),
    FUNCTION_TABLE_ENTRY(vkDestroyBuffer, true),
    FUNCTION_TABLE_ENTRY(vkCreateBufferView, true),
    FUNCTION_TABLE_ENTRY(vkDestroyBufferView, true),
    FUNCTION_TABLE_ENTRY(vkCreateImage, true),
    FUNCTION_TABLE_ENTRY(vkDestroyImage, true),
    FUNCTION_TABLE_ENTRY(vkGetImageSubresourceLayout, true),
    FUNCTION_TABLE_ENTRY(vkCreateImageView, true),
    FUNCTION_TABLE_ENTRY(vkDestroyImageView, true),
    FUNCTION_TABLE_ENTRY(vkCreateShaderModule, true),
    FUNCTION_TABLE_ENTRY(vkDestroyShaderModule, true),
    FUNCTION_TABLE_ENTRY(vkCreatePipelineCache, true),
    FUNCTION_TABLE_ENTRY(vkDestroyPipelineCache, true),
    FUNCTION_TABLE_ENTRY(vkGetPipelineCacheData, true),
    FUNCTION_TABLE_ENTRY(vkMergePipelineCaches, true),
    FUNCTION_TABLE_ENTRY(vkCreateGraphicsPipelines, true),
    FUNCTION_TABLE_ENTRY(vkCreateComputePipelines, true),
    FUNCTION_TABLE_ENTRY(vkDestroyPipeline, true),
    FUNCTION_TABLE_ENTRY(vkCreatePipelineLayout, true),
    FUNCTION_TABLE_ENTRY(vkDestroyPipelineLayout, true),
    FUNCTION_TABLE_ENTRY(vkCreateSampler, true),
    FUNCTION_TABLE_ENTRY(vkDestroySampler, true),
    FUNCTION_TABLE_ENTRY(vkCreateDescriptorSetLayout, true),
    FUNCTION_TABLE_ENTRY(vkDestroyDescriptorSetLayout, true),
    FUNCTION_TABLE_ENTRY(vkCreateDescriptorPool, true),
    FUNCTION_TABLE_ENTRY(vkDestroyDescriptorPool, true),
    FUNCTION_TABLE_ENTRY(vkResetDescriptorPool, true),
    FUNCTION_TABLE_ENTRY(vkAllocateDescriptorSets, true),
    FUNCTION_TABLE_ENTRY(vkFreeDescriptorSets, true),
    FUNCTION_TABLE_ENTRY(vkUpdateDescriptorSets, true),
    FUNCTION_TABLE_ENTRY(vkCreateFramebuffer, true),
    FUNCTION_TABLE_ENTRY(vkDestroyFramebuffer, true),
    FUNCTION_TABLE_ENTRY(vkCreateRenderPass, true),
    FUNCTION_TABLE_ENTRY(vkDestroyRenderPass, true),
    FUNCTION_TABLE_ENTRY(vkGetRenderAreaGranularity, true),
    FUNCTION_TABLE_ENTRY(vkCreateCommandPool, true),
    FUNCTION_TABLE_ENTRY(vkDestroyCommandPool, true),
    FUNCTION_TABLE_ENTRY(vkResetCommandPool, true),
    FUNCTION_TABLE_ENTRY(vkAllocateCommandBuffers, true),
    FUNCTION_TABLE_ENTRY(vkFreeCommandBuffers, true),
    FUNCTION_TABLE_ENTRY(vkBeginCommandBuffer, true),
    FUNCTION_TABLE_ENTRY(vkEndCommandBuffer, true),
    FUNCTION_TABLE_ENTRY(vkResetCommandBuffer, true),
    FUNCTION_TABLE_ENTRY(vkCmdBindPipeline, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetViewport, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetScissor, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetLineWidth, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetDepthBias, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetBlendConstants, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetDepthBounds, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetStencilCompareMask, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetStencilWriteMask, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetStencilReference, true),
    FUNCTION_TABLE_ENTRY(vkCmdBindDescriptorSets, true),
    FUNCTION_TABLE_ENTRY(vkCmdBindIndexBuffer, true),
    FUNCTION_TABLE_ENTRY(vkCmdBindVertexBuffers, true),
    FUNCTION_TABLE_ENTRY(vkCmdDraw, true),
    FUNCTION_TABLE_ENTRY(vkCmdDrawIndexed, true),
    FUNCTION_TABLE_ENTRY(vkCmdDrawIndirect, true),
    FUNCTION_TABLE_ENTRY(vkCmdDrawIndexedIndirect, true),
    FUNCTION_TABLE_ENTRY(vkCmdDispatch, true),
    FUNCTION_TABLE_ENTRY(vkCmdDispatchIndirect, true),
    FUNCTION_TABLE_ENTRY(vkCmdCopyBuffer, true),
    FUNCTION_TABLE_ENTRY(vkCmdCopyImage, true),
    FUNCTION_TABLE_ENTRY(vkCmdBlitImage, true),
    FUNCTION_TABLE_ENTRY(vkCmdCopyBufferToImage, true),
    FUNCTION_TABLE_ENTRY(vkCmdCopyImageToBuffer, true),
    FUNCTION_TABLE_ENTRY(vkCmdUpdateBuffer, true),
    FUNCTION_TABLE_ENTRY(vkCmdFillBuffer, true),
    FUNCTION_TABLE_ENTRY(vkCmdClearColorImage, true),
    FUNCTION_TABLE_ENTRY(vkCmdClearDepthStencilImage, true),
    FUNCTION_TABLE_ENTRY(vkCmdClearAttachments, true),
    FUNCTION_TABLE_ENTRY(vkCmdResolveImage, true),
    FUNCTION_TABLE_ENTRY(vkCmdSetEvent, true),
    FUNCTION_TABLE_ENTRY(vkCmdResetEvent, true),
    FUNCTION_TABLE_ENTRY(vkCmdWaitEvents, true),
    FUNCTION_TABLE_ENTRY(vkCmdPipelineBarrier, true),
    FUNCTION_TABLE_ENTRY(vkCmdBeginQuery, true),
    FUNCTION_TABLE_ENTRY(vkCmdEndQuery, true),
    FUNCTION_TABLE_ENTRY(vkCmdResetQueryPool, true),
    FUNCTION_TABLE_ENTRY(vkCmdWriteTimestamp, true),
    FUNCTION_TABLE_ENTRY(vkCmdCopyQueryPoolResults, true),
    FUNCTION_TABLE_ENTRY(vkCmdPushConstants, true),
    FUNCTION_TABLE_ENTRY(vkCmdBeginRenderPass, true),
    FUNCTION_TABLE_ENTRY(vkCmdNextSubpass, true),
    FUNCTION_TABLE_ENTRY(vkCmdEndRenderPass, true),
    FUNCTION_TABLE_ENTRY(vkCmdExecuteCommands, true),

    FUNCTION_TABLE_ENTRY(vkDestroySurfaceKHR, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceSurfaceSupportKHR, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceSurfaceFormatsKHR, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceSurfacePresentModesKHR, true),

#if defined(Y_PLATFORM_WINDOWS)
    FUNCTION_TABLE_ENTRY(vkCreateWin32SurfaceKHR, true),
    FUNCTION_TABLE_ENTRY(vkGetPhysicalDeviceWin32PresentationSupportKHR, true)
#endif
};

static void ResetVulkanLibraryFunctionPointers()
{
    vkCreateInstance = nullptr;
}

#if defined(Y_PLATFORM_WINDOWS)

static HMODULE s_hVulkanModule;
static Y_ATOMIC_DECL uint32 s_vulkanLibraryRefCount = 0;

bool LoadVulkanLibrary()
{
    // Not thread safe if a second thread calls the loader whilst the first is still in-progress.
    if (s_hVulkanModule != nullptr)
    {
        Y_AtomicIncrement(s_vulkanLibraryRefCount);
        return true;
    }

    s_hVulkanModule = LoadLibraryA("vulkan-1.dll");
    if (s_hVulkanModule == nullptr)
    {
        Log_ErrorPrint("Failed to load vulkan-1.dll");
        return false;
    }

    for (size_t i = 0; i < countof(s_functionTableEntries); i++)
    {
        FARPROC functionAddress = GetProcAddress(s_hVulkanModule, s_functionTableEntries[i].FunctionName);
        *s_functionTableEntries[i].FunctionPointerAddress = reinterpret_cast<void*>(functionAddress);

        if (functionAddress == nullptr && s_functionTableEntries[i].Required)
        {
            Log_ErrorPrintf("Failed to load required function %s", s_functionTableEntries[i].FunctionName);
            ResetVulkanLibraryFunctionPointers();
            FreeLibrary(s_hVulkanModule);
            s_hVulkanModule = nullptr;
            return false;
        }
    }

    Y_AtomicIncrement(s_vulkanLibraryRefCount);
    return true;
}

void UnloadVulkanLibrary()
{
    if (Y_AtomicDecrement(s_vulkanLibraryRefCount) > 0)
        return;

    ResetVulkanLibraryFunctionPointers();
    FreeLibrary(s_hVulkanModule);
    s_hVulkanModule = nullptr;
}

#endif
