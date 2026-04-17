#include "frame_buffers.h"

VkResult createFrameBuffer(VkRenderPass* renderPass, std::vector<VkImageView>& imageViews, VkDevice* logicalDevice, VkExtent2D* extend, std::vector<VkFramebuffer>& frameBuffers){

    VkFramebufferCreateInfo frameBufferCreateInfo {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.renderPass = *renderPass;
    frameBufferCreateInfo.attachmentCount = 1;
    frameBufferCreateInfo.width = extend->width;
    frameBufferCreateInfo.height = extend->height;
    frameBufferCreateInfo.layers = 1;

    for(VkImageView imageView : imageViews){
        VkFramebuffer frameBuffer;
        frameBufferCreateInfo.pAttachments = &imageView;
        VkResult createFrameBufferResult = vkCreateFramebuffer(*logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffer);
        if(createFrameBufferResult != VK_SUCCESS){
            return createFrameBufferResult;
        }
        frameBuffers.push_back(frameBuffer);
    }
    return VK_SUCCESS;
}

