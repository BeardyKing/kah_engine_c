#ifndef GFX_VULKAN_BINDLESS_H
#define GFX_VULKAN_BINDLESS_H

//===API======================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===GLOABL_DEFINES============================================================
#define KAH_SAMPLER_ID_NEAREST 0
#define KAH_SAMPLER_ID_LINEAR 1
//=============================================================================

//===API=======================================================================
void gfx_bindless_set_sampler(uint32_t samplerIndex, VkSampler sampler);
void gfx_bindless_set_image(uint32_t imageIndex, VkImageView imageView);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_bindless_create();
void gfx_bindless_cleanup();
//=============================================================================

#endif //GFX_VULKAN_BINDLESS_H
