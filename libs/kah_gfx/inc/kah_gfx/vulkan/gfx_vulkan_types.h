#ifndef GFX_VULKAN_TYPES_H
#define GFX_VULKAN_TYPES_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>

#include <kah_math/vec2.h>
//=============================================================================

//===POOL_TYPES================================================================
struct GfxImage {
    VkImage image;
    VkImageView view;
    VmaAllocation alloc;
} typedef GfxImage;
//=============================================================================

//===PUBLIC_TYPES==============================================================
struct GlobalGfx{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    VmaAllocator allocator;
    VkAllocationCallbacks* allocationCallbacks;
    VkSampleCountFlagBits sampleCount;
}typedef GlobalGfx; //extern GlobalGfx g_gfx;

enum GfxSizeType {
    GFX_SIZE_TYPE_ABSOLUTE,
    GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE,
}typedef GfxSizeType;

struct GfxAttachmentInfo {
    VkFormat format;
    GfxSizeType sizeType;
    vec2u size;
}typedef GfxAttachmentInfo;
//=============================================================================

#endif //GFX_VULKAN_TYPES_H
