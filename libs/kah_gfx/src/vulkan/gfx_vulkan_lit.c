//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_lit.h>

#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
#include <kah_gfx/vulkan/gfx_vulkan_bindless.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_pipeline_builder.h>
#include <kah_gfx/vulkan/gfx_vulkan_utils.h>
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===PRIVATE_STRUCTS===========================================================
#define GFX_LIT_BINDING_SIZE 2
#define GFX_LIT_BINDING_LAYOUT_SIZE 1

#define GFX_LIT_BINDLESS_RESOURCE_MAX 1000
#define GFX_LIT_BINDLESS_SAMPLER_MAX 8

#define GFX_LIT_BINDING_TEXTURE_INDEX 0
#define GFX_LIT_BINDING_SAMPLER_INDEX 1

struct PushConstants{
    uint32_t bindlessImageIndex;
} typedef PushConstants;

static struct VulkanLit {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
} s_gfxLit = {};
//=============================================================================

//===API=======================================================================
void gfx_lit_draw(VkCommandBuffer cmdBuffer){
    VkDescriptorSet bindlessSet = gfx_bindless_get_descriptor_set();
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_gfxLit.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS, s_gfxLit.pipelineLayout,0, 1, &bindlessSet, 0, NULL);
    vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_lit_create() {
    VkDescriptorSetLayout bindlessSet = gfx_bindless_get_descriptor_set_layout();
    VkPushConstantRange bufferRange = (VkPushConstantRange){
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(PushConstants),
    };
    s_gfxLit.pipelineLayout = vk_pipeline_layout_create(&bindlessSet, 1, &bufferRange, 1);

    //TODO: cache off color/depth or pass into func.
    const VkSurfaceFormatKHR colorFormat = gfx_vulkan_utils_select_surface_format();
    const VkFormat depthFormat = gfx_vulkan_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);

    PipelineBuilder builder = gfx_pipeline_builder_create(s_gfxLit.pipelineLayout);
    gfx_pipeline_builder_set_input_topology(&builder, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    gfx_pipeline_builder_set_polygon_mode(&builder, VK_POLYGON_MODE_FILL);
    // gfx_pipeline_builder_set_cull_enabled(&builder);
    gfx_pipeline_builder_set_cull_disabled(&builder);
    gfx_pipeline_builder_set_multisampling(&builder, VK_SAMPLE_COUNT_1_BIT);
    gfx_pipeline_builder_set_blending_disabled(&builder);
    gfx_pipeline_builder_set_color_attachment_format(&builder, colorFormat.format);
    gfx_pipeline_builder_set_depth_stencil_attachment_format(&builder, depthFormat);
    // gfx_pipeline_builder_set_depth_test_enabled(&builder, true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    gfx_pipeline_builder_set_depth_test_disabled(&builder);
    s_gfxLit.pipeline = gfx_pipeline_builder_build(&builder, "assets/shaders/lit/lit.vert", "assets/shaders/lit/lit.frag");
}

void gfx_lit_cleanup() {
    vkDestroyPipelineLayout(g_gfx.device, s_gfxLit.pipelineLayout, g_gfx.allocationCallbacks);
    vkDestroyPipeline(g_gfx.device, s_gfxLit.pipeline, g_gfx.allocationCallbacks);
}
//=============================================================================