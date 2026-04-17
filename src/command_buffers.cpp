#include "command_buffers.h"

VkResult allocateCommandBuffers(VkDevice* logicalDevice, VkCommandPool* commandPool, int imageViewsSize, std::vector<VkCommandBuffer>& commandBuffers){
    VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = *commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = imageViewsSize;

    VkResult allocateCommandBuffersResult = vkAllocateCommandBuffers(*logicalDevice, &commandBufferAllocateInfo, commandBuffers.data());
    if(allocateCommandBuffersResult != VK_SUCCESS){
        return allocateCommandBuffersResult;
    }

    return VK_SUCCESS;
    
}