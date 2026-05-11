#include "descriptor_set.h"

VkResult createDescriptorLayout(VkDevice* logicalDevice, VkDescriptorSetLayout* descriptorSetLayout){

    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding {};
    descriptorSetLayoutBinding.binding = 0;
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1;
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

    return vkCreateDescriptorSetLayout(*logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, descriptorSetLayout);
}

VkResult createDescriptorPool(VkDevice* logicalDevice, VkDescriptorPool* descriptorPool){
    VkDescriptorPoolSize descriptorPoolSize {};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

    return vkCreateDescriptorPool(*logicalDevice, &descriptorPoolCreateInfo, nullptr, descriptorPool);
}

VkResult allocateDescriptorSet(VkDevice* logicalDevice, VkDescriptorPool* descriptorPool, VkDescriptorSetLayout* descriptorSetLayout, VkDescriptorSet* descriptorSet){
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = *descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayout;

    return vkAllocateDescriptorSets(*logicalDevice, &descriptorSetAllocateInfo, descriptorSet);
}

void updateUniformBuffer(VkDevice* logicalDevice, VkDescriptorSet* descriptorSet, VkBuffer* uniformBuffer){
    
    VkDescriptorBufferInfo descriptorBufferInfo {};
    descriptorBufferInfo.buffer = *uniformBuffer;
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet writeDescriptorSet {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = *descriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
    
    vkUpdateDescriptorSets(*logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
}