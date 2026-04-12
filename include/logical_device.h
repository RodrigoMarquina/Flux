#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult selectLogicalDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkDevice* logicalDevice, VkQueue* queueHandle);