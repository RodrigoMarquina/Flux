#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult allocateCommandBuffers(VkDevice* logicalDevice, VkCommandPool* commandPool, int imageViewsSize, std::vector<VkCommandBuffer>& commandBuffers);
