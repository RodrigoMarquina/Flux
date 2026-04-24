#include "window.h"
#include "vulkan_instance.h"
#include "vulkan_surface.h"
#include "physical_device.h"
#include "logical_device.h"
#include "swapchain.h"
#include "swapchain_images.h"
#include "render_pass.h"
#include "frame_buffers.h"
#include "command_pool.h"
#include "command_buffers.h"
#include "shader_module.h"
#include "pipeline.h"
#include "sync_objects.h"
#include "vertex_buffer.h"
#include "depth_buffer.h"

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
	
	VkSwapchainKHR swapchain;
	VkFormat format;
	VkExtent2D extent;
	VkResult swapchainResult = createSwapchain(&swapchain, &format, &physicalDevice, &logicalDevice, &surface, queueFamilyIndex, window, &extent);
	if(swapchainResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Swapchain.\n");
		fprintf(stderr, "VkResult code: %d\n", swapchainResult);
		return 1;
	}

	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	VkResult getSwapchainImagesResult = getSwapchainImages(&swapchain, &logicalDevice, images, imageViews, &format);
	if(getSwapchainImagesResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Swapchain images.\n");
		fprintf(stderr, "VkResult code: %d\n", getSwapchainImagesResult);
		return 1;
	}

	VkRenderPass renderPass;
	VkResult createRenderPassResult = createRenderPass(&logicalDevice, &format, &renderPass);
	if(createRenderPassResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Render pass.\n");
		fprintf(stderr, "VkResult code: %d\n", createRenderPassResult);
		return 1;
	}

	VkImage depthImage;
	VkDeviceMemory depthDeviceMemory;
	VkImageView depthImageView; 
	VkResult createDepthBufferResult = createDepthBuffer(&physicalDevice, &logicalDevice, &extent, &depthImage, &depthDeviceMemory, &depthImageView);
	if(createDepthBufferResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Depth Image.\n");
		fprintf(stderr, "VkResult code: %d\n", createDepthBufferResult);
		return 1;
	}

	std::vector<VkFramebuffer> frameBuffers;
	VkResult createFrameBufferResult = createFrameBuffer(&renderPass, imageViews, &logicalDevice, &extent, frameBuffers, &depthImageView);
	if(createFrameBufferResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Frame Buffer.\n");
		fprintf(stderr, "VkResult code: %d\n", createFrameBufferResult);
		return 1;
	}

	VkCommandPool commandPool;
	VkResult createCommandPoolResult = createCommandPool(&logicalDevice, queueFamilyIndex, &commandPool);
	if(createCommandPoolResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Command Pool.\n");
		fprintf(stderr, "VkResult code: %d\n", createCommandPoolResult);
		return 1;
	}

	std::vector<VkCommandBuffer> commandBuffers(imageViews.size());
	VkResult allocateCommandBuffersResult = allocateCommandBuffers(&logicalDevice, &commandPool, imageViews.size(), commandBuffers);
	if(allocateCommandBuffersResult != VK_SUCCESS){
		fprintf(stderr, "Failed to allocate Command Buffers.\n");
		fprintf(stderr, "VkResult code: %d\n", allocateCommandBuffersResult);
		return 1;
	}

	//Vert file
	VkShaderModule shaderModuleVert;
	VkResult createShaderModuleResultVert = createShaderModule(&logicalDevice, &shaderModuleVert, "/home/kina/Documents/Flux/shaders/cube.vert.spv");
	if(createShaderModuleResultVert != VK_SUCCESS){
		fprintf(stderr, "Failed to create Shader Module for .vert file.\n");
		fprintf(stderr, "VkResult code: %d\n", createShaderModuleResultVert);
		return 1;
	}

	//Frag file
	VkShaderModule shaderModuleFrag;
	VkResult createShaderModuleResultFrag = createShaderModule(&logicalDevice, &shaderModuleFrag, "/home/kina/Documents/Flux/shaders/cube.frag.spv");
	if(createShaderModuleResultFrag != VK_SUCCESS){
		fprintf(stderr, "Failed to create Shader Module for .frag file.\n");
		fprintf(stderr, "VkResult code: %d\n", createShaderModuleResultFrag);
		return 1;
	}

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkViewport viewport;
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissors;
	scissors.offset = {0, 0};
	scissors.extent = extent;
	VkResult createPipelineResult = createPipeline(&pipeline, &renderPass, &logicalDevice, &pipelineLayout, &shaderModuleVert, &shaderModuleFrag, &viewport, &scissors);
	if(createPipelineResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Pipeline.\n");
		fprintf(stderr, "VkResult code: %d\n", createPipelineResult);
		return 1;
	}

	std::vector<VkSemaphore> imageSemaphores(imageViews.size());
	std::vector<VkSemaphore> renderSemaphores(imageViews.size());
	VkFence frameFence;
	VkResult syncObjectsResult = syncObjects(imageSemaphores, renderSemaphores, &frameFence, imageViews.size(), &logicalDevice);
	if(syncObjectsResult != VK_SUCCESS){
		fprintf(stderr, "Failed to sync objects.\n");
		fprintf(stderr, "VkResult code: %d\n", syncObjectsResult);
		return 1;
	}

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexDeviceMemory;
	VkResult createVertexBufferResult = createVertexBuffer(&logicalDevice, &physicalDevice, &vertexBuffer, &vertexDeviceMemory);
	if(createVertexBufferResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Vertex Buffer.\n");
		fprintf(stderr, "VkResult code: %d\n", createVertexBufferResult);
		return 1;
	}

	VkBuffer indexBuffer;
	VkDeviceMemory indexDeviceMemory;
	VkResult createIndexBufferResult = createIndexBuffer(&logicalDevice, &physicalDevice, &indexBuffer, &indexDeviceMemory);
	if(createIndexBufferResult != VK_SUCCESS){
		fprintf(stderr, "Failed to create Index Buffer.\n");
		fprintf(stderr, "VkResult code: %d\n", createIndexBufferResult);
		return 1;
	}

	uint32_t imageIndex = 0;
	uint32_t currentFrame = 0;
	VkDeviceSize deviceSize = 0;
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0] = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[1] = {1.0f, 0};
	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		vkWaitForFences(logicalDevice, 1, &frameFence, VK_TRUE, UINT64_MAX); //Infinite wait
		vkResetFences(logicalDevice, 1, &frameFence);
		vkAcquireNextImageKHR(logicalDevice, swapchain, UINT64_MAX, imageSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex); //Takes either a semaphore OR a fence to signal
		vkResetCommandBuffer(commandBuffers[imageIndex], 0);
		VkCommandBufferBeginInfo commandBufferBeginInfo {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffers[imageIndex], &commandBufferBeginInfo);
		VkRenderPassBeginInfo renderPassBeginInfo {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = frameBuffers[imageIndex];
		renderPassBeginInfo.renderArea = scissors;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, &vertexBuffer, &deviceSize);
		vkCmdBindIndexBuffer(commandBuffers[imageIndex], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(commandBuffers[imageIndex], 36, 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		vkEndCommandBuffer(commandBuffers[imageIndex]);
		VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1; //Render only needs to wait on imageSemaphore
		submitInfo.pWaitSemaphores = &imageSemaphores[currentFrame];
		VkFlags pipelineStageFlags {};
		pipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &pipelineStageFlags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderSemaphores[imageIndex];
		vkQueueSubmit(queueHandle, 1, &submitInfo, frameFence);
		VkPresentInfoKHR presentInfoKHR {};
		presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfoKHR.waitSemaphoreCount = 1; //Present only needs to wait on renderSemaphore
		presentInfoKHR.pWaitSemaphores = &renderSemaphores[imageIndex];
		presentInfoKHR.swapchainCount = 1;
		presentInfoKHR.pSwapchains = &swapchain;
		presentInfoKHR.pImageIndices = &imageIndex;
		vkQueuePresentKHR(queueHandle, &presentInfoKHR);
		currentFrame = (currentFrame + 1) % imageSemaphores.size();
	}

	vkDeviceWaitIdle(logicalDevice); //Waits for GPU work to finish before destroying anything

	vkDestroyImage(logicalDevice, depthImage, nullptr);
	vkFreeMemory(logicalDevice, depthDeviceMemory, nullptr);
	vkDestroyImageView(logicalDevice, depthImageView, nullptr);
	vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
	vkFreeMemory(logicalDevice, indexDeviceMemory, nullptr);
	vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
	vkFreeMemory(logicalDevice, vertexDeviceMemory, nullptr);
	for(VkSemaphore imageSemaphore : imageSemaphores){
		vkDestroySemaphore(logicalDevice, imageSemaphore, nullptr);
	}
	for(VkSemaphore renderSemaphore : renderSemaphores){
		vkDestroySemaphore(logicalDevice, renderSemaphore, nullptr);
	}
	vkDestroyFence(logicalDevice, frameFence, nullptr);
	vkDestroyPipeline(logicalDevice, pipeline, nullptr);
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
	vkDestroyShaderModule(logicalDevice, shaderModuleVert, nullptr);
	vkDestroyShaderModule(logicalDevice, shaderModuleFrag, nullptr);
	vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
	for(VkFramebuffer frameBuffer : frameBuffers){
		vkDestroyFramebuffer(logicalDevice, frameBuffer, nullptr);
	}
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
	for(VkImageView imageView : imageViews){
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}
	vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}