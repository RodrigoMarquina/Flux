#include "render_pass.h"

VkResult createRenderPass(VkDevice* logicalDevice, VkFormat* format, VkRenderPass* renderPass){
    VkAttachmentDescription attachmentDescription {};
    attachmentDescription.format = *format;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachmentReference {};
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;

    VkRenderPassCreateInfo renderPassCreateInfo {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.subpassCount = 1;

    VkResult createRenderPassResult = vkCreateRenderPass(*logicalDevice, &renderPassCreateInfo, nullptr, renderPass);

    return createRenderPassResult;
}

