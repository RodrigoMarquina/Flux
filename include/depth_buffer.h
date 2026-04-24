#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>

VkResult createDepthBuffer(VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkExtent2D* extent, VkImage* depthImage, VkDeviceMemory* depthDeviceMemory, VkImageView* depthImageView);
