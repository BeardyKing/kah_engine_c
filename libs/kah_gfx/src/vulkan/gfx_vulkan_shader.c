//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_shader.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
#include <kah_gfx/gfx_converter.h>

#include <kah_core/assert.h>
#include <kah_core/allocators.h>
#include <kah_core/file_io.h>
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===API=======================================================================
VkShaderModule gfx_shader_load_binary(const char *path){
    AllocInfo* allocInfo = file_io_load_into_buffer(allocators()->arena, path, true);

    VkShaderModuleCreateInfo moduleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = allocInfo->commitedMemory,
        .pCode = (const uint32_t *)allocInfo->bufferAddress,
    };

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    const VkResult moduleResult = vkCreateShaderModule( g_gfx.device, &moduleCreateInfo, NULL, &shaderModule );
    core_assert(moduleResult == VK_SUCCESS);
    return shaderModule;
}

VkPipelineShaderStageCreateInfo gfx_shader_load(const char *path, VkShaderStageFlagBits stage){
#if CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)
    gfx_convert_shader_spv(path);
#endif

    VkPipelineShaderStageCreateInfo shaderStage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = stage,
        .module = gfx_shader_load_binary(path),
        .pName = "main",
        .pSpecializationInfo = NULL,
    };

    core_assert(shaderStage.module != VK_NULL_HANDLE);
    return shaderStage;
}
//=============================================================================