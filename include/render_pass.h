#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult createRenderPass(VkDevice* logicalDevice, VkFormat* format, VkRenderPass* renderPass);