#include <kah_gfx/vulkan/gfx_vulkan_buffer.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>

#include <kah_core/assert.h>

//===INTERNAL_STRUCTS===================================================================================================
extern GlobalGfx g_gfx;
//======================================================================================================================

//===API=======================================================================
GfxBuffer gfx_buffer_create(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    const VkBufferCreateInfo bufferInfo = (VkBufferCreateInfo){
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = allocSize,
        .usage = usage,
    };

    const VmaAllocationCreateInfo allocInfo = (VmaAllocationCreateInfo){
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = memoryUsage,
    };

    GfxBuffer buffer = (GfxBuffer){};
    VkResult bufferRes = vmaCreateBuffer( g_gfx.allocator, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, &buffer.info);
    core_assert(bufferRes == VK_SUCCESS);

    if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0) {
        const VkBufferDeviceAddressInfo deviceAdressInfo = (VkBufferDeviceAddressInfo){
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer.buffer,
        };
        buffer.address = vkGetBufferDeviceAddress(g_gfx.device, &deviceAdressInfo);
        buffer.usingBufferDeviceAddress = true;
    }

    return buffer;
}

void gfx_buffer_cleanup(GfxBuffer* inBuffer){
    core_assert(inBuffer);
    vmaDestroyBuffer(g_gfx.allocator, inBuffer->buffer, inBuffer->allocation);
    *inBuffer = (GfxBuffer){};
}
//=============================================================================