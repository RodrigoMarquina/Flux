#include "command_pool.h"

VkResult createCommandPool(VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPool* commandPool){
    VkCommandPoolCreateInfo commandPoolCreateInfo {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    VkResult createCommandPoolResult = vkCreateCommandPool(*logicalDevice, &commandPoolCreateInfo, nullptr, commandPool);
    if(createCommandPoolResult != VK_SUCCESS){
        return createCommandPoolResult;
    }
    
    return createCommandPoolResult;
}