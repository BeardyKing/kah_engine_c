#ifndef GFX_VULKAN_TYPES_H
#define GFX_VULKAN_TYPES_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===PUBLIC_TYPES==============================================================
struct GfxVulkanTargetAttachmentFormats {
    VkSurfaceFormatKHR surfaceFormat;
    VkFormat depthFormat;
    VkFormat colorFormat;
} typedef GfxVulkanTargetAttachmentFormats;
//=============================================================================

#endif //GFX_VULKAN_TYPES_H
