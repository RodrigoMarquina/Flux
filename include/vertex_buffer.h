#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>

VkResult createVertexBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkBuffer* vertexBuffer, VkDeviceMemory* vertexDeviceMemory);

VkResult createIndexBuffer(VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, VkBuffer* indexBuffer, VkDeviceMemory* indexDeviceMemory);
