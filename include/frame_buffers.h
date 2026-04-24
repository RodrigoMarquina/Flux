#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <array>

VkResult createFrameBuffer(VkRenderPass* renderPass, std::vector<VkImageView>& imageViews, VkDevice* logicalDevice, VkExtent2D* extend, std::vector<VkFramebuffer>& frameBuffers, VkImageView* depthImageView);
