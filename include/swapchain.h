#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <algorithm>

VkResult createSwapchain(VkSwapchainKHR* swapchain,VkFormat* format, VkPhysicalDevice* physicalDevice, VkDevice* logicalDevice, VkSurfaceKHR* surface, GLFWwindow* window, VkExtent2D* extent);
