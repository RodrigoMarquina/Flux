#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult selectPhysicalDevice(VkInstance* instance, VkSurfaceKHR* surface, VkPhysicalDevice* physicalDevice, uint32_t* queueFamilyIndex);
