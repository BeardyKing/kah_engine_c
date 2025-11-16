#ifndef GFX_VULKAN_TYPES_H
#define GFX_VULKAN_TYPES_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_buffer.h>

#include <kah_core/texture_formats.h>

#include <kah_math/vec2.h>
//=============================================================================

//===GLOBAL_DEFINES============================================================
constexpr uint32_t KAH_SWAP_CHAIN_IMAGE_COUNT = 2;
constexpr uint32_t KAH_BUFFER_COUNT = 2;
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

struct GfxTexture {
    VkImage image; // TODO: consider replacing with GfxImageHandle
    VkImageView imageView;
    VmaAllocation allocation;
    VkFormat format;
    VkImageUsageFlags usage;
    VkExtent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    uint32_t bindlessIndex;
#if KAH_DEBUG
    char debug_name[128]; //TODO: replace with gfx string arena.
#endif
}typedef GfxTexture;
typedef uint32_t GfxTextureHandle;

struct GfxMesh{
    GfxBuffer vertexBuffer;
    GfxBuffer indexBuffer;
    uint32_t numVertices;
    uint32_t numIndices;
} typedef GfxMesh;
typedef uint32_t GfxMeshHandle;
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
    GFX_RESOURCE_NONE                   = 0,
    GFX_RESOURCE_IMAGE_COLOR            = 1 << 0,
    GFX_RESOURCE_IMAGE_DEPTH_STENCIL    = 2 << 0,
    // GFX_RESOURCE_BUFFER              = 4 << 0,
    GFX_RESOURCE_IMAGE_EXTERNAL_CB      = 5 << 0,
}typedef GfxResourceType;

#define TG_INVALID UINT32_MAX

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
