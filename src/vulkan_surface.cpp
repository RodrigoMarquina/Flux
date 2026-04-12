#include "vulkan_surface.h"

VkResult createVulkanSurface(VkInstance* instance, GLFWwindow* window, VkSurfaceKHR* surface){
    
    VkResult result = glfwCreateWindowSurface(*instance, window, NULL, surface);

    return result;
}