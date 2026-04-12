#include "window.h"
#include "vulkan_instance.h"
#include "vulkan_surface.h"
#include "physical_device.h"
#include "logical_device.h"

int main(){
	if(!glfwInit()){
        fprintf(stderr, "Error: Couldnt load GLFW.\n");
		return 1;
    }

	GLFWwindow* window = createWindow(640, 480, "Flux", NULL);
	if(!window){
		fprintf(stderr, "Failed to create window.\n");
		return 1;
	}

	VkInstance instance;
	VkResult instanceResult = createVulkanInstance(&instance);
	if(instanceResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create instance.\n");
		fprintf(stderr, "VkResult code: %d\n", instanceResult);
		return 1;
	}

	VkSurfaceKHR surface;
	VkResult surfaceResult = createVulkanSurface(&instance, window, &surface);
	if(surfaceResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create surface.\n");
		fprintf(stderr, "VkResult code: %d\n", surfaceResult);
		return 1;
	}

	VkPhysicalDevice physicalDevice;
	uint32_t queueFamilyIndex = 0;
	VkResult physicalDeviceResult = selectPhysicalDevice(&instance, &surface, &physicalDevice, &queueFamilyIndex);
	if(physicalDeviceResult != VK_SUCCESS){
		fprintf(stderr, "Failed to find device.\n");
		fprintf(stderr, "VkResult code: %d\n", physicalDeviceResult);
		return 1;
	}

	VkDevice logicalDevice;
	VkQueue queueHandle;
	VkResult logicalDeviceResult = selectLogicalDevice(physicalDevice, queueFamilyIndex, &logicalDevice, &queueHandle);
	if(logicalDeviceResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create logical device.\n");
		fprintf(stderr, "VkResult code: %d\n", logicalDeviceResult);
		return 1;
	}

	fprintf(stdout, "Logical device created successfully.\n");

	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
    }
	
	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}