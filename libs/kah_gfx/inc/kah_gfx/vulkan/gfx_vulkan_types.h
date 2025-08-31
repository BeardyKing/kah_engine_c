#ifndef GFX_VULKAN_TYPES_H
#define GFX_VULKAN_TYPES_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===POOL_TYPES================================================================
#define GFX_POOL_GFX_IMAGE_COUNT_MAX UINT16_MAX
struct GfxImage {
    VkImage image;
    VkImageView view;
    VmaAllocation alloc;
} typedef GfxImage;
//=============================================================================

//===PUBLIC_TYPES==============================================================
struct GfxVulkanTargetAttachmentFormats {
    VkSurfaceFormatKHR surfaceFormat;
    VkFormat depthFormat;
    VkFormat colorFormat;
} typedef GfxVulkanTargetAttachmentFormats;
//=============================================================================

#endif //GFX_VULKAN_TYPES_H
