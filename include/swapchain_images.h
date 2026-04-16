#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult getSwapchainImages(VkSwapchainKHR* swapchain, VkDevice* logicalDevice, std::vector<VkImage>& images, std::vector<VkImageView>& imageViews, VkFormat* format);
