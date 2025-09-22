#include <kah_gfx/vulkan/gfx_vulkan_lit.h>

#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_shader.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_lit_create() {
    // Tests on demand shader compiler.
    constexpr uint32_t LIT_SHADER_STAGE_COUNT = 2;
    VkPipelineShaderStageCreateInfo shaderStages[LIT_SHADER_STAGE_COUNT] = {};

    shaderStages[0] = gfx_shader_load("assets/shaders/lit/lit.vert", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = gfx_shader_load("assets/shaders/lit/lit.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
    vkDestroyShaderModule(g_gfx.device, shaderStages[0].module, g_gfx.allocationCallbacks);
    vkDestroyShaderModule(g_gfx.device, shaderStages[1].module, g_gfx.allocationCallbacks);
}

void gfx_lit_cleanup() {

}
//=============================================================================