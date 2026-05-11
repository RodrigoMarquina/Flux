#pragma once

#include <vulkan/vulkan.h>

VkResult createDescriptorLayout(VkDevice* logicalDevice, VkDescriptorSetLayout* descriptorSetLayout);

VkResult createDescriptorPool(VkDevice* logicalDevice, VkDescriptorPool* descriptorPool);

VkResult allocateDescriptorSet(VkDevice* logicalDevice, VkDescriptorPool* descriptorPool, VkDescriptorSetLayout* descriptorSetLayout, VkDescriptorSet* descriptorSet);

void updateUniformBuffer(VkDevice* logicalDevice, VkDescriptorSet* descriptorSet, VkBuffer* uniformBuffer);
