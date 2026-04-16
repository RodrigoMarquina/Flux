#include "swapchain_images.h"

VkResult getSwapchainImages(VkSwapchainKHR* swapchain, VkDevice* logicalDevice, std::vector<VkImage>& images, std::vector<VkImageView>& imageViews, VkFormat* format){
    uint32_t imageCount = 0;
    VkResult result1 = vkGetSwapchainImagesKHR(*logicalDevice, *swapchain, &imageCount, nullptr);
    if(result1 != VK_SUCCESS){
		return result1;
	}
    if(imageCount == 0){
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    images.resize(imageCount);
    vkGetSwapchainImagesKHR(*logicalDevice, *swapchain, &imageCount, images.data());

    for(VkImage image : images){
        VkImageView imageView;
        VkImageViewCreateInfo imageViewCreateInfo {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = *format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        
        VkResult createImageViewResult = vkCreateImageView(*logicalDevice, &imageViewCreateInfo, nullptr, &imageView);
        if(createImageViewResult != VK_SUCCESS){
		    return createImageViewResult;
	    }
        imageViews.push_back(imageView);
    }
    return result1;
}
