#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <array>

VkResult createPipeline(VkPipeline* pipeline, VkRenderPass* renderPass, VkDevice* logicalDevice, VkPipelineLayout* pipelineLayout, VkShaderModule* shaderModuleVert, VkShaderModule* shaderModuleFrag, VkViewport* viewport, VkRect2D* scissors);
