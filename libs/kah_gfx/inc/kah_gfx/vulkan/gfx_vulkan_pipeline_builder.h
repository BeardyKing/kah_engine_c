#ifndef GFX_PIPELINE_BUILDER_H
#define GFX_PIPELINE_BUILDER_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===PUBLIC_STRUCTS============================================================
struct PipelineBuilder{
    VkPipelineLayout pipelineLayout;
    VkFormat colorAttachmentformat;
    VkPipelineRenderingCreateInfo renderInfo;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
    VkPipelineRasterizationStateCreateInfo rasterizerState;
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
    VkPipelineMultisampleStateCreateInfo multisamplingState;
    VkPipelineDepthStencilStateCreateInfo depthStencilState;
} typedef PipelineBuilder;
//=============================================================================

//===API=======================================================================
PipelineBuilder gfx_pipeline_builder_create(VkPipelineLayout pipelineLayout);
VkPipeline gfx_pipeline_builder_build(PipelineBuilder* builder, const char* vertPath, const char* fragPath);

void gfx_pipeline_builder_set_input_topology(PipelineBuilder* builder, VkPrimitiveTopology topology);
void gfx_pipeline_builder_set_polygon_mode(PipelineBuilder* builder, VkPolygonMode mode);
void gfx_pipeline_builder_set_cull_mode(PipelineBuilder* builder, VkCullModeFlags cullMode, VkFrontFace frontFace);
void gfx_pipeline_builder_set_cull_disabled(PipelineBuilder* builder);
void gfx_pipeline_builder_set_cull_enabled(PipelineBuilder* builder);
void gfx_pipeline_builder_set_multisampling(PipelineBuilder* builder, VkSampleCountFlagBits sampleCount);
void gfx_pipeline_builder_set_blending_disabled(PipelineBuilder* builder);
void gfx_pipeline_builder_set_blending_enabled(PipelineBuilder* builder, VkBlendOp blendOp, VkBlendFactor srcBlendFactor, VkBlendFactor dstBlendFactor, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor);
void gfx_pipeline_builder_set_blending_enabled_default(PipelineBuilder* builder);
void gfx_pipeline_builder_set_color_attachment_format(PipelineBuilder* builder, VkFormat format);
void gfx_pipeline_builder_set_depth_stencil_attachment_format(PipelineBuilder* builder, VkFormat format);
void gfx_pipeline_builder_set_depth_test_disabled(PipelineBuilder* builder);
void gfx_pipeline_builder_set_depth_test_enabled(PipelineBuilder* builder, bool depthWriteEnable, VkCompareOp op);
//=============================================================================

#endif //GFX_PIPELINE_BUILDER_H
