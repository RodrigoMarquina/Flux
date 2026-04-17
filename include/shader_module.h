#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>

VkResult createShaderModule(VkDevice* logicalDevice, VkShaderModule* shaderModule, std::string path);
