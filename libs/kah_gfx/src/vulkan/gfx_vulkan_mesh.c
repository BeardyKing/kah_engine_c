//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_mesh.h>
#include <kah_gfx/vulkan/gfx_vulkan_buffer.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
//=============================================================================

//===API=======================================================================
GfxMesh gfx_mesh_load_from_memory(CoreRawMesh rawMesh)
{
    GfxMesh outMesh = (GfxMesh){};
    const uint32_t indexBufferSize = rawMesh.indexCount * sizeof(uint32_t);
    outMesh.indexBuffer = gfx_buffer_create(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_AUTO);

    const uint32_t vertexBufferSize = rawMesh.vertexCount * sizeof(CoreVertex);
    outMesh.vertexBuffer = gfx_buffer_create(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_AUTO);

    GfxBuffer staging = gfx_buffer_create(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO);
    {
        VkCommandBuffer cmdBuffer = gfx_command_buffer_start_immediate_recording();
        {
            const VkBufferCopy vertexCopy = (VkBufferCopy){ .srcOffset = 0, .dstOffset = 0, .size = vertexBufferSize,};
            vkCmdCopyBuffer(cmdBuffer, staging.buffer, outMesh.vertexBuffer.buffer, 1, &vertexCopy);

            const VkBufferCopy indexCopy = (VkBufferCopy){ .srcOffset = vertexBufferSize, .dstOffset = 0, .size = indexBufferSize};
            vkCmdCopyBuffer(cmdBuffer, staging.buffer, outMesh.indexBuffer.buffer, 1, &indexCopy);
        }
        gfx_command_buffer_end_immediate_recording(cmdBuffer);
    }
    gfx_buffer_cleanup(&staging);
    return outMesh;
}

GfxMesh gfx_mesh_build_plane()
{
    CoreVertex v0 = {.pos = {{-0.5f, -0.5f, 0.0f}}, .normal = {}, .uv = {}, .color = {{1.0f, 0.0f, 0.0f}}};
    CoreVertex v1 = {.pos = {{0.5f, -0.5f, 0.0f}}, .normal = {}, .uv = {}, .color = {{0.0f, 1.0f, 0.0f}}};
    CoreVertex v2 = {.pos = {{0.5f, 0.5f, 0.0f}}, .normal = {}, .uv = {}, .color = {{0.0f, 0.0f, 1.0f}}};
    CoreVertex v3 = {.pos = {{-0.5f, 0.5f, 0.0f}}, .normal = {}, .uv = {}, .color = {{1.0f, 1.0f, 1.0f}}};

    constexpr uint32_t PLANE_VERTEX_COUNT = 4;
    CoreVertex vertexData[PLANE_VERTEX_COUNT] = {v0, v1, v2, v3};

    constexpr uint32_t PLANE_INDEX_COUNT = 6;
    uint32_t indexData[PLANE_INDEX_COUNT] = {
        0, 1, 2, 2, 3, 0
    };

    CoreRawMesh rawMesh = {
        .vertexData = &vertexData[0],
        .indexData = &indexData[0],
        .vertexCount = PLANE_VERTEX_COUNT,
        .indexCount = PLANE_INDEX_COUNT,
    };

    return gfx_mesh_load_from_memory(rawMesh);
}

void gfx_mesh_cleanup(GfxMesh* inMesh){
    gfx_buffer_cleanup(&inMesh->vertexBuffer);
    gfx_buffer_cleanup(&inMesh->indexBuffer);
    *inMesh = (GfxMesh){};
}
//=============================================================================
