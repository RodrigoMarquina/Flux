#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

VkResult createFrameBuffer(VkRenderPass* renderPass, std::vector<VkImageView>& imageViews, VkDevice* logicalDevice, VkExtent2D* extend, std::vector<VkFramebuffer>& frameBuffers);
