#ifndef GFX_VULKAN_UBO_H
#define GFX_VULKAN_UBO_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_buffer.h>
//=============================================================================

//===GLOABL_DEFINES============================================================
#define KAH_DESCRIPTOR_SET_BINDING_UBO_SCENE 0
//=============================================================================

//===API=======================================================================
GfxBuffer* gfx_uniform_buffer_scene_get();
VkDescriptorSet gfx_uniform_buffer_scene_descriptor_set();
VkDescriptorSetLayout gfx_uniform_buffer_scene_descriptor_set_layout();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_uniform_buffers_create();
void gfx_uniform_buffers_cleanup();
//=============================================================================

#endif //GFX_VULKAN_UBO_H
