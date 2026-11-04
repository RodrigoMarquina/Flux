#include "physical_device.h"

VkResult selectPhysicalDevice(VkInstance* instance, VkSurfaceKHR* surface, VkPhysicalDevice* physicalDevice, uint32_t* queueFamilyIndex){
    uint32_t count = 0;
    VkResult result1 = vkEnumeratePhysicalDevices(*instance, &count, nullptr);
    if(result1 != VK_SUCCESS){
		return result1;
	}
    if(count == 0){
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    std::vector<VkPhysicalDevice> deviceList(count);
    VkResult result2 = vkEnumeratePhysicalDevices(*instance, &count, deviceList.data());

    for(VkPhysicalDevice dev : deviceList){
        uint32_t propertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &propertyCount, nullptr);
        std::vector<VkQueueFamilyProperties> devProperties(propertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &propertyCount, devProperties.data());

        for(uint32_t i = 0; i < devProperties.size(); i++){
            if(devProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
                VkBool32 support = false;
                VkResult supportsSurface = vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, *surface, &support);
                if(supportsSurface != VK_SUCCESS){
                    fprintf(stderr, "Surface support query failed.\n");
                    fprintf(stderr, "VkResult code: %d\n", supportsSurface);
                    return supportsSurface;
                }
                if(support){
                    *physicalDevice = dev;
                    *queueFamilyIndex = i;
                    return result2;
                }
            }
        }
    }

    return VK_ERROR_INITIALIZATION_FAILED;
}