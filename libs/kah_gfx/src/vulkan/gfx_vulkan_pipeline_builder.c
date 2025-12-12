//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_pipeline_builder.h>

#include <kah_gfx/vulkan/gfx_vulkan_shader.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>

#include <kah_core/assert.h>
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===API=======================================================================
PipelineBuilder gfx_pipeline_builder_create(VkPipelineLayout pipelineLayout){
    PipelineBuilder builder = (PipelineBuilder){
        .pipelineLayout = pipelineLayout,
        .colorAttachmentformat = VK_FORMAT_UNDEFINED,
        .renderInfo = (VkPipelineRenderingCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR},

        .inputAssemblyState = (VkPipelineInputAssemblyStateCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO},
        .rasterizerState = (VkPipelineRasterizationStateCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO},
        .colorBlendAttachmentState = (VkPipelineColorBlendAttachmentState){},
        .multisamplingState = (VkPipelineMultisampleStateCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO},
        .depthStencilState = (VkPipelineDepthStencilStateCreateInfo){.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO},
    };
    return builder;
}

void gfx_pipeline_builder_set_input_topology(PipelineBuilder* builder, VkPrimitiveTopology topology){
    builder->inputAssemblyState.topology = topology;
    builder->inputAssemblyState.primitiveRestartEnable = VK_FALSE;
}

void gfx_pipeline_builder_set_polygon_mode(PipelineBuilder* builder, VkPolygonMode mode){
    builder->rasterizerState.polygonMode = mode;
    builder->rasterizerState.lineWidth = 1.f;
}

void gfx_pipeline_builder_set_cull_mode(PipelineBuilder* builder, VkCullModeFlags cullMode, VkFrontFace frontFace){
    builder->rasterizerState.cullMode = cullMode;
    builder->rasterizerState.frontFace = frontFace;
}

void gfx_pipeline_builder_set_cull_disabled(PipelineBuilder* builder){
    gfx_pipeline_builder_set_cull_mode(builder, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
}

void gfx_pipeline_builder_set_cull_enabled(PipelineBuilder* builder){
    gfx_pipeline_builder_set_cull_mode(builder, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
}

void gfx_pipeline_builder_set_multisampling(PipelineBuilder* builder, VkSampleCountFlagBits sampleCount){
    builder->multisamplingState.sampleShadingEnable = VK_FALSE;
    builder->multisamplingState.rasterizationSamples = sampleCount;
    builder->multisamplingState.minSampleShading = 1.0f;
    builder->multisamplingState.alphaToCoverageEnable = VK_FALSE;
    builder->multisamplingState.alphaToOneEnable = VK_FALSE;
}

void gfx_pipeline_builder_set_blending_disabled(PipelineBuilder* builder){
    builder->colorBlendAttachmentState.blendEnable = VK_FALSE;
    builder->colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
}

void gfx_pipeline_builder_set_blending_enabled(PipelineBuilder* builder, VkBlendOp blendOp, VkBlendFactor srcBlendFactor, VkBlendFactor dstBlendFactor, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor){
    builder->colorBlendAttachmentState.blendEnable = VK_TRUE;
    builder->colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    builder->colorBlendAttachmentState.srcColorBlendFactor = srcBlendFactor;
    builder->colorBlendAttachmentState.dstColorBlendFactor = dstBlendFactor;
    builder->colorBlendAttachmentState.colorBlendOp = blendOp;
    builder->colorBlendAttachmentState.srcAlphaBlendFactor = srcAlphaBlendFactor;
    builder->colorBlendAttachmentState.dstAlphaBlendFactor = dstAlphaBlendFactor;
    builder->colorBlendAttachmentState.alphaBlendOp = blendOp;
}

void gfx_pipeline_builder_set_blending_enabled_default(PipelineBuilder* builder){
    gfx_pipeline_builder_set_blending_enabled(builder, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
}

void gfx_pipeline_builder_set_color_attachment_format(PipelineBuilder* builder, VkFormat format){
    builder->colorAttachmentformat = format;
    builder->renderInfo.colorAttachmentCount = 1;
    builder->renderInfo.pColorAttachmentFormats = &builder->colorAttachmentformat;
}

void gfx_pipeline_builder_set_depth_stencil_attachment_format(PipelineBuilder* builder, VkFormat format){
    builder->renderInfo.depthAttachmentFormat = format;
    builder->renderInfo.stencilAttachmentFormat = format;
}

void gfx_pipeline_builder_set_depth_test_disabled(PipelineBuilder* builder){
    builder->depthStencilState.depthTestEnable = VK_FALSE;
    builder->depthStencilState.depthWriteEnable = VK_FALSE;
    builder->depthStencilState.depthCompareOp = VK_COMPARE_OP_NEVER;
    builder->depthStencilState.depthBoundsTestEnable = VK_FALSE;
    builder->depthStencilState.stencilTestEnable = VK_FALSE;
    builder->depthStencilState.front = (VkStencilOpState){};
    builder->depthStencilState.back = (VkStencilOpState){};
    builder->depthStencilState.minDepthBounds = 0.f;
    builder->depthStencilState.maxDepthBounds = 1.f;
}

void gfx_pipeline_builder_set_depth_test_enabled(PipelineBuilder* builder, bool depthWriteEnable, VkCompareOp op){
    builder->depthStencilState.depthTestEnable = VK_TRUE;
    builder->depthStencilState.depthWriteEnable = depthWriteEnable;
    builder->depthStencilState.depthCompareOp = op;
    builder->depthStencilState.depthBoundsTestEnable = VK_FALSE;
    builder->depthStencilState.stencilTestEnable = VK_FALSE;
    builder->depthStencilState.front = (VkStencilOpState){};
    builder->depthStencilState.back = (VkStencilOpState){};
    builder->depthStencilState.minDepthBounds = 0.f;
    builder->depthStencilState.maxDepthBounds = 1.f;
}


VkPipeline gfx_pipeline_builder_build(PipelineBuilder* builder, const char* vertPath, const char* fragPath){

    const VkPipelineViewportStateCreateInfo viewportState = (VkPipelineViewportStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    const VkPipelineColorBlendStateCreateInfo colorBlending = (VkPipelineColorBlendStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &builder->colorBlendAttachmentState,
    };

    const VkPipelineVertexInputStateCreateInfo vertexInputInfo = (VkPipelineVertexInputStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    const VkDynamicState dynamicStates[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    const VkPipelineDynamicStateCreateInfo dynamicInfo = (VkPipelineDynamicStateCreateInfo){
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = &dynamicStates[0],
    };

    const VkPipelineShaderStageCreateInfo shaderStages[2] = {
        gfx_shader_load(vertPath,VK_SHADER_STAGE_VERTEX_BIT),
        gfx_shader_load(fragPath, VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    const VkGraphicsPipelineCreateInfo pipelineInfo = (VkGraphicsPipelineCreateInfo){
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &builder->renderInfo,
        .stageCount = 2,
        .pStages = &shaderStages[0],
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &builder->inputAssemblyState,
        .pViewportState = &viewportState,
        .pRasterizationState = &builder->rasterizerState,
        .pMultisampleState = &builder->multisamplingState,
        .pDepthStencilState = &builder->depthStencilState,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicInfo,
        .layout = builder->pipelineLayout,
    };

    VkPipeline outPipeline = VK_NULL_HANDLE;
    VkResult result = vkCreateGraphicsPipelines(g_gfx.device, VK_NULL_HANDLE, 1, &pipelineInfo, g_gfx.allocationCallbacks, &outPipeline);
    core_assert(result == VK_SUCCESS);

    vkDestroyShaderModule(g_gfx.device, shaderStages[0].module, g_gfx.allocationCallbacks);
    vkDestroyShaderModule(g_gfx.device, shaderStages[1].module, g_gfx.allocationCallbacks);

    return outPipeline;
}
//=============================================================================