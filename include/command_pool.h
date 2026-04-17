#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult createCommandPool(VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPool* commandPool);
