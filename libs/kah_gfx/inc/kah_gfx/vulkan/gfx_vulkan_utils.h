#ifndef GFX_VULKAN_UTILS_H
#define GFX_VULKAN_UTILS_H
//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===API=======================================================================
const char* VkImageLayout_c_str( VkImageLayout imageLayout );
const char* VkAccessFlagBits_c_str( VkAccessFlagBits accessFlagBits );
const char* VkPipelineStageFlags_c_str( VkPipelineStageFlags pipelineStageFlags );
//=============================================================================

#endif //GFX_VULKAN_UTILS_H
