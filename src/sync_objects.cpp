#include "sync_objects.h"

VkResult syncObjects(std::vector<VkSemaphore>& imageSemaphores, std::vector<VkSemaphore>& renderSemaphores, VkFence* frameFence, uint32_t imageViewsSize, VkDevice* logicalDevice){
    VkSemaphoreCreateInfo semaphoreCreateInfo {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for(uint32_t i = 0; i < imageViewsSize; i++){
        VkSemaphore imageSemaphore;
        VkResult createSemaphoreResultImage = vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &imageSemaphore);
        if(createSemaphoreResultImage != VK_SUCCESS){
            return createSemaphoreResultImage;
        }
        imageSemaphores[i] = imageSemaphore;
    }
    for(uint32_t i = 0; i < imageViewsSize; i++){
        VkSemaphore renderSemaphore;
        VkResult createSemaphoreResultRender = vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &renderSemaphore);
        if(createSemaphoreResultRender != VK_SUCCESS){
            return createSemaphoreResultRender;
        }
        renderSemaphores[i] = renderSemaphore;
    }

    VkFenceCreateInfo fenceCreateInfo {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkResult createFenceResult = vkCreateFence(*logicalDevice, &fenceCreateInfo, nullptr, frameFence);
    if(createFenceResult != VK_SUCCESS){
        return createFenceResult;
    }

    return VK_SUCCESS;
}
