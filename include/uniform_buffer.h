#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct UniformBuffer{
        glm::mat4 Model = glm::mat4(1.0);
        glm::mat4 View = glm::mat4(1.0);
        glm::mat4 Projection = glm::mat4(1.0);
};

VkResult createUniformBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkBuffer* uniformBuffer, VkDeviceMemory* uniformDeviceMemory, void** memoryMap);
