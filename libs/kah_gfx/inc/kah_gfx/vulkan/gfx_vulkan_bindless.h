#ifndef GFX_VULKAN_BINDLESS_H
#define GFX_VULKAN_BINDLESS_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===GLOABL_DEFINES============================================================
#define KAH_SAMPLER_ID_NEAREST 0
#define KAH_SAMPLER_ID_LINEAR 1

#define KAH_DESCRIPTOR_SET_BINDING_BINLESS_TEXTURE 0
#define KAH_DESCRIPTOR_SET_BINDING_BINLESS_SAMPLER 1
//=============================================================================

//===API=======================================================================
void gfx_bindless_set_sampler(uint32_t samplerIndex, VkSampler sampler);
void gfx_bindless_set_image(uint32_t imageIndex, VkImageView imageView);

VkDescriptorSetLayout gfx_bindless_get_descriptor_set_layout();
VkDescriptorSet gfx_bindless_get_descriptor_set();
VkSampler gfx_get_sampler_linear();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_bindless_create();
void gfx_bindless_cleanup();
//=============================================================================

#endif //GFX_VULKAN_BINDLESS_H
