#pragma once
#include "YBaseLib/Log.h"
#include "YRenderLib/Vulkan/VulkanCommon.h"

const char* VkResultToString(VkResult res);

void LogVulkanResult(const char* channelName, const char* functionName, LOGLEVEL level, VkResult res, const char* msg, ...);

#define LOG_VULKAN_ERROR(res, ...) LogVulkanResult("VulkanBackend", __FUNCTION__, LOGLEVEL_ERROR, res, __VA_ARGS__)

