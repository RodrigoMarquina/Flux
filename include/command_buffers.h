#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult allocateCommandBuffers(VkDevice* logicalDevice, VkCommandPool* commandPool, uint32_t imageViewsSize, std::vector<VkCommandBuffer>& commandBuffers);
