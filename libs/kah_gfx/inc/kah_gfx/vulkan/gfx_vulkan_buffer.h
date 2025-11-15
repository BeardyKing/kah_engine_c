#ifndef GFX_VULKAN_BUFFER_H
#define GFX_VULKAN_BUFFER_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===PUBLIC_TYPES==============================================================
struct GfxBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;

    bool usingBufferDeviceAddress;
    VkDeviceAddress address;
} typedef GfxBuffer;
//=============================================================================

//===API=======================================================================
GfxBuffer gfx_buffer_create(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
void gfx_buffer_cleanup(GfxBuffer* inBuffer);
//=============================================================================

#endif //GFX_VULKAN_BUFFER_H
