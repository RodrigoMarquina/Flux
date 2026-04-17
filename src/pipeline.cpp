#include "pipeline.h"
#include <vulkan/vulkan.h>
#include <array>

VkResult createPipeline(VkPipeline* pipeline, VkRenderPass* renderPass, VkDevice* logicalDevice, VkPipelineLayout* pipelineLayout, VkShaderModule* shaderModuleVert, VkShaderModule* shaderModuleFrag, VkViewport* viewport, VkRect2D* scissors){
    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfoVert {};
    pipelineShaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineShaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineShaderStageCreateInfoVert.module = *shaderModuleVert;
    pipelineShaderStageCreateInfoVert.pName = "main";
    pipelineShaderStageCreateInfoVert.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfoFrag {};
    pipelineShaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineShaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineShaderStageCreateInfoFrag.module = *shaderModuleFrag;
    pipelineShaderStageCreateInfoFrag.pName = "main";
    pipelineShaderStageCreateInfoFrag.pSpecializationInfo = nullptr;

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStage = {pipelineShaderStageCreateInfoVert, pipelineShaderStageCreateInfoFrag};

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo {};
    pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo {};
    pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
    
    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo {};
    pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportStateCreateInfo.viewportCount = 1;
    pipelineViewportStateCreateInfo.pViewports = viewport;
    pipelineViewportStateCreateInfo.scissorCount = 1;
    pipelineViewportStateCreateInfo.pScissors = scissors;

    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo {};
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo {};
    pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState {};
    pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo {};
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    pipelineColorBlendStateCreateInfo.attachmentCount = 1;
    pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

	VkResult createPipelineLayoutResult = vkCreatePipelineLayout(*logicalDevice, &pipelineLayoutCreateInfo, nullptr, pipelineLayout);
    if(createPipelineLayoutResult != VK_SUCCESS){
        return createPipelineLayoutResult;
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStage.data();
    graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.layout = *pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = *renderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkResult createGraphicsPipelinesResult = vkCreateGraphicsPipelines(*logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, pipeline); 
    if(createGraphicsPipelinesResult != VK_SUCCESS){
        return createGraphicsPipelinesResult;
    }

    return VK_SUCCESS;
}