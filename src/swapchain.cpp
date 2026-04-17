#include "swapchain.h"

VkResult createSwapchain(VkSwapchainKHR* swapchain, VkFormat* format, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkSurfaceKHR* surface, uint32_t queueFamilyIndex, GLFWwindow* window, VkExtent2D* extent){
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    VkResult physicalDeviceSurfaceCapabilitiesResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*physicalDevice, *surface, &surfaceCapabilities);
    if(physicalDeviceSurfaceCapabilitiesResult != VK_SUCCESS){
		return physicalDeviceSurfaceCapabilitiesResult;
	}

    //__Format__
    uint32_t surfaceFormatCount = 0;
    VkResult physicalDeviceSurfaceFormatResult1 = vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, *surface, &surfaceFormatCount, nullptr);
    if(physicalDeviceSurfaceFormatResult1 != VK_SUCCESS){
		return physicalDeviceSurfaceFormatResult1;
	}
    if(surfaceFormatCount == 0){
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    std::vector<VkSurfaceFormatKHR> surfaceFormatList(surfaceFormatCount);
    VkResult physicalDeviceSurfaceFormatResult2 = vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, *surface, &surfaceFormatCount, surfaceFormatList.data());
    if(physicalDeviceSurfaceFormatResult2 != VK_SUCCESS){
		return physicalDeviceSurfaceFormatResult2;
	}

    //Format & Colorspace
    *format = surfaceFormatList[0].format;
    VkColorSpaceKHR colorSpace = surfaceFormatList[0].colorSpace;
    for(int i = 0; i < surfaceFormatList.size(); i++){
        if(surfaceFormatList[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormatList[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            *format = surfaceFormatList[i].format;
            colorSpace = surfaceFormatList[i].colorSpace;
        }
    }

    //__Present Modes__
    uint32_t presentModeCount = 0;
    VkResult physicalDeviceSurfaceModesResult1 = vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, *surface, &presentModeCount, nullptr);
    if(physicalDeviceSurfaceModesResult1 != VK_SUCCESS){
		return physicalDeviceSurfaceModesResult1;
	}
    if(presentModeCount == 0){
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    std::vector<VkPresentModeKHR> presentModeList(presentModeCount);
    VkResult physicalDeviceSurfaceModesResult2 = vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, *surface, &presentModeCount, presentModeList.data());
    if(physicalDeviceSurfaceModesResult2 != VK_SUCCESS){
		return physicalDeviceSurfaceModesResult2;
	} 

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(int i = 0; i < presentModeList.size(); i++){
        if(presentModeList[i] == VK_PRESENT_MODE_MAILBOX_KHR){
            presentMode = presentModeList[i];
        }
    }

    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    if(surfaceCapabilities.currentExtent.width == UINT32_MAX){
        extent->width = std::clamp(static_cast<uint32_t>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
    } else{
        extent->width = surfaceCapabilities.currentExtent.width;
    }
    if(surfaceCapabilities.currentExtent.height == UINT32_MAX){
        extent->height = std::clamp(static_cast<uint32_t>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    } else{
        extent->height = surfaceCapabilities.currentExtent.height;
    }

    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = *surface;
    swapchainInfo.imageFormat = *format; 
    if(surfaceCapabilities.maxImageCount != 0 && surfaceCapabilities.minImageCount + 1 > surfaceCapabilities.maxImageCount){
        swapchainInfo.minImageCount = surfaceCapabilities.minImageCount; //+1 so we ensure that min is always at least 2
    } else{
        swapchainInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
    }
    swapchainInfo.imageColorSpace = colorSpace;
    swapchainInfo.imageExtent = {extent->width, extent->height};
    swapchainInfo.imageArrayLayers = 1; //More than 1 is for stereoscopic/VR
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //One queue family owns it
    swapchainInfo.queueFamilyIndexCount = 0; //When exclusive
    swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //No window Transparency
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE; //Do not render pixels hidden by other windows
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult createSwapchainResult = vkCreateSwapchainKHR(*logicalDevice, &swapchainInfo, nullptr, swapchain);

    return createSwapchainResult;
}