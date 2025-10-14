#ifndef GFX_VULKAN_UTILS_H
#define GFX_VULKAN_UTILS_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
//=============================================================================

//===API=======================================================================
const char* VkImageLayout_c_str( VkImageLayout imageLayout );
const char* VkAccessFlagBits_c_str( VkAccessFlagBits accessFlagBits );
const char* VkPipelineStageFlags_c_str( VkPipelineStageFlags pipelineStageFlags );

VkFormat gfx_utils_core_image_format_to_vk( CoreTextureFormat textureFormat );
uint32_t gfx_utils_get_memory_type(VkPhysicalDeviceMemoryProperties deviceMemoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);
//=============================================================================

#endif //GFX_VULKAN_UTILS_H
