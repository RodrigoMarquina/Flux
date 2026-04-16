#include "logical_device.h"

VkResult selectLogicalDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkDevice* logicalDevice, VkQueue* queueHandle){
    float queuePriority = 1.0f;
    const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceQueueCreateInfo queueCreateInfo{};

    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo{};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = extensions;
    deviceCreateInfo.pEnabledFeatures = nullptr;

    VkResult createDeviceResult = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, logicalDevice);
    if(createDeviceResult != VK_SUCCESS){
        return createDeviceResult;
    }

    vkGetDeviceQueue(*logicalDevice, queueFamilyIndex, 0, queueHandle);

    return createDeviceResult;
}