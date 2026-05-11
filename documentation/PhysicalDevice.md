Physical device is the abstract definition Vulkan gives to the hardware devices that the system has.

Each manufacturer when creating the device, define what it can do in a industry standard identificator called
queuefamilies.

To find a good candidate for our application, we need to iterate through all the physical devices the system
has an check their queuefamilies to see what that one in particular can do. Some examples are rendering and compute.
The Vulkan loader communicates with the driver at runtime creating a bridge between the hardware and the program.

Here we are doing that, we first call the vkEnumeratePhysicalDevices function to get the amount of physical devices 
our system has, then we create a vector with the size of the devices, then we call the function again and fill
the vectors with the devices.

---
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
---

Then we look the vector and check each device for the queuefamilies we need. First we once again get the amount
of queuefamilies by calling the vkGetPhysicalDeviceQueueFamilyProperties twice and in the second one filling
a vector with the queues. Then we iterate that vector and check if it has the queuefamily we want. In this case
we are looking for the VK_QUEUE_GRAPHICS_BIT flag. if we find it, we check if that device supports the surface
in the same queue. That way we avoid performance issues by trying to asyncronate between two different queues.
Finally we return the physical device that has all the features we need.

---
for(VkPhysicalDevice dev : deviceList){
    uint32_t propertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &propertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> devProperties(propertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &propertyCount, devProperties.data());

    for(int i = 0; i < devProperties.size(); i++){
        if(devProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            VkBool32 support = false;
            VkResult supportsSurface = vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, *surface, &support);
            if(support){
                *physicalDevice = dev;
                *queueFamilyIndex = i;
                return result2;
            }
        }
    }
}

return VK_ERROR_INITIALIZATION_FAILED;
---


 