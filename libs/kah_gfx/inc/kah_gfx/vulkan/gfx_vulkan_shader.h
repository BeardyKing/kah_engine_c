#ifndef GFX_VULKAN_SHADER_H
#define GFX_VULKAN_SHADER_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===API=======================================================================
VkShaderModule gfx_shader_load_binary(const char *path);
VkPipelineShaderStageCreateInfo gfx_shader_load(const char *path, VkShaderStageFlagBits stage);
//=============================================================================


#endif //GFX_VULKAN_SHADER_H
