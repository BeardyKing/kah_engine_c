#include <kah_gfx/vulkan/gfx_vulkan_shader.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
#include <kah_gfx/gfx_converter.h>

#include <kah_core/assert.h>
#include <kah_core/allocators.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===API=======================================================================
VkShaderModule gfx_shader_load_binary(const char *path){
    int fd = open(path, O_RDONLY | O_BINARY);
    core_assert_msg(fd >= 0,"Err: Failed to open shader file");

    struct stat st;
    core_assert_msg(fstat(fd, &st) == 0, "Err: fstat failed");

    size_t size = st.st_size;
    core_assert(size > 0);

    AllocInfo* alloc = allocators()->arena.alloc(size);

    char *shaderCode = (char *)alloc->bufferAddress;
    ssize_t bytesRead = read(fd, shaderCode, size);
    close(fd);

    core_assert_msg(bytesRead == (ssize_t)size,"Err: Failed to read shader file");
    VkShaderModuleCreateInfo moduleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = size,
        .pCode = (const uint32_t *)shaderCode,
    };

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkResult moduleResult = vkCreateShaderModule(
        g_gfx.device, &moduleCreateInfo, NULL, &shaderModule
    );
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