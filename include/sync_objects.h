#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult syncObjects(std::vector<VkSemaphore>& imageSemaphores, std::vector<VkSemaphore>& renderSemaphores, VkFence* frameFence, int imageViewsSize, VkDevice* logicalDevice);