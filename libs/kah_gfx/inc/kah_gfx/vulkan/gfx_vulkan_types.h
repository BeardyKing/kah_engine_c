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
    vec2u size;
} typedef GfxImage;
typedef uint32_t GfxImageHandle;
typedef GfxImage (*GfxImageExternal_cb)();
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

enum GfxResourceType : uint8_t{
    GFX_RESOURCE_NONE,
    GFX_RESOURCE_IMAGE_COLOR,
    GFX_RESOURCE_IMAGE_DEPTH_STENCIL,
    // GFX_RESOURCE_BUFFER,
    GFX_RESOURCE_IMAGE_EXTERNAL_CB,
}typedef GfxResourceType;

struct GfxResource{
    GfxResourceType type;
    union{
        struct {
            uint32_t binding;
            GfxImageHandle handle;
        }imageColor;
        struct {
            GfxImageHandle handle;
        }imageDepthStencil;
        struct{
            uint32_t binding;
            GfxImageExternal_cb imageCB ;
        }external;
    } data;
}typedef GfxResource;

constexpr uint32_t GFX_RENDER_CONTEXT_READ_WRITE_MAX = 4;
struct GfxRenderContext{
    GfxResource read[GFX_RENDER_CONTEXT_READ_WRITE_MAX];
    GfxResource write[GFX_RENDER_CONTEXT_READ_WRITE_MAX];
    uint32_t readCount;
    uint32_t writeCount;
}typedef GfxRenderContext;

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
