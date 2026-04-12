#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

VkResult createVulkanSurface(VkInstance* instance, GLFWwindow* window ,VkSurfaceKHR* surface);