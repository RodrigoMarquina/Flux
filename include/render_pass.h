#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <array>

VkResult createRenderPass(VkDevice* logicalDevice, VkFormat* format, VkRenderPass* renderPass);