//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_lit.h>

#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
#include <kah_gfx/vulkan/gfx_vulkan_bindless.h>
#include <kah_gfx/vulkan/gfx_vulkan_uniform_buffers.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_pipeline_builder.h>
#include <kah_gfx/vulkan/gfx_vulkan_utils.h>

#include <kah_gfx/gfx_pool.h>

#include "kah_gfx/vulkan/gfx_vulkan_mesh.h"
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===PRIVATE_STRUCTS===========================================================
struct PushConstants{
    mat4f model;
    uint32_t bindlessImageIndex;
} typedef PushConstants;

static struct VulkanLit {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
} s_gfxLit = {};
//=============================================================================

//===API=======================================================================
void gfx_lit_draw(VkCommandBuffer cmdBuffer){
    VkDescriptorSet descriptorSets[2] = {
        gfx_bindless_get_descriptor_set(),          // (set = 0, binding = 0) & (set = 0, binding = 1)
        gfx_uniform_buffer_scene_descriptor_set(),  // (set = 1, binding = 0)
    };

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_gfxLit.pipeline);
    vkCmdBindDescriptorSets(cmdBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS, s_gfxLit.pipelineLayout,0, 2, &descriptorSets[0], 0, NULL);

    LitEntityHandle entId = GFX_POOL_NULL_HANDLE;
    while ((entId = gfx_pool_lit_entity_get_next_active(entId)) != GFX_POOL_NULL_HANDLE) {
        LitEntity* ent = gfx_pool_lit_entity_get(entId);
        Transform* transform = gfx_pool_transform_get(ent->transformIndex);
        GfxMesh* mesh = gfx_pool_gfx_mesh_get(ent->meshIndex);
        LitMaterial* material = gfx_pool_lit_material_get(ent->materialIndex);

        transform->rotation = (vec3f){transform->rotation.x + 3, transform->rotation.y + 3, transform->rotation.z + 3};

        PushConstants pushConstants = (PushConstants){
            .model = transform_model_matrix_quat_cast(transform),
            .bindlessImageIndex = material->albedoImageIndex,
        };
        vkCmdPushConstants( cmdBuffer, s_gfxLit.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &pushConstants);

        const VkBuffer vertexBuffers[] = {mesh->vertexBuffer.buffer};
        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(cmdBuffer, mesh->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuffer, mesh->numIndices, 1, 0, 0, 0);
    }
}

//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_lit_create() {
    VkPushConstantRange bufferRange = (VkPushConstantRange){
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(PushConstants),
    };
    constexpr uint32_t LIT_DESC_LAYOUT_COUNT = 2;
    VkDescriptorSetLayout descriptorLayouts[LIT_DESC_LAYOUT_COUNT];
    descriptorLayouts[0] = gfx_bindless_get_descriptor_set_layout();
    descriptorLayouts[1] = gfx_uniform_buffer_scene_descriptor_set_layout();

    s_gfxLit.pipelineLayout = vk_pipeline_layout_create(&descriptorLayouts[0], LIT_DESC_LAYOUT_COUNT, &bufferRange, 1);

    //TODO: cache off color/depth or pass into func.
    const VkSurfaceFormatKHR colorFormat = gfx_vulkan_utils_select_surface_format();
    const VkFormat depthFormat = gfx_vulkan_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);

    //TODO: replace with vertexBuffer
    VkVertexInputBindingDescription bindingDesc = (VkVertexInputBindingDescription){ 0, sizeof(CoreVertex), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attributeDescs[4] = {
        (VkVertexInputAttributeDescription){0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(CoreVertex, pos)},
        (VkVertexInputAttributeDescription){1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(CoreVertex, normal)},
        (VkVertexInputAttributeDescription){2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(CoreVertex, uv)},
        (VkVertexInputAttributeDescription){3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(CoreVertex, color)},
    };

    PipelineBuilder builder = gfx_pipeline_builder_create(s_gfxLit.pipelineLayout);
    gfx_pipeline_builder_set_input_topology(&builder, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    gfx_pipeline_builder_set_polygon_mode(&builder, VK_POLYGON_MODE_FILL);
    // gfx_pipeline_builder_set_cull_enabled(&builder);
    gfx_pipeline_builder_set_cull_disabled(&builder);
    gfx_pipeline_builder_set_multisampling(&builder, VK_SAMPLE_COUNT_1_BIT);
    gfx_pipeline_builder_set_blending_disabled(&builder);
    gfx_pipeline_builder_set_color_attachment_format(&builder, colorFormat.format);
    gfx_pipeline_builder_set_depth_stencil_attachment_format(&builder, depthFormat);
    // gfx_pipeline_builder_set_depth_test_enabled(&builder, true, VK_COMPARE_OP_LESS_OR_EQUAL);
    gfx_pipeline_builder_set_depth_test_disabled(&builder);
    gfx_pipeline_builder_set_vertex_input(&builder, &bindingDesc, 1, attributeDescs, 4);
    s_gfxLit.pipeline = gfx_pipeline_builder_build(&builder, "assets/shaders/lit/lit.vert", "assets/shaders/lit/lit.frag");
}

void gfx_lit_cleanup() {
    vkDestroyPipeline(g_gfx.device, s_gfxLit.pipeline, g_gfx.allocationCallbacks);
    vkDestroyPipelineLayout(g_gfx.device, s_gfxLit.pipelineLayout, g_gfx.allocationCallbacks);
}
//=============================================================================