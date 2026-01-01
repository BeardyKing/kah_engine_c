//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_mesh.h>
#include <kah_gfx/vulkan/gfx_vulkan_buffer.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
#include <kah_gfx/gfx_pool.h>
//=============================================================================

//===API=======================================================================
GfxMeshHandle gfx_mesh_load_from_memory(CoreRawMesh rawMesh){
    const GfxMeshHandle outMeshHandle = gfx_pool_gfx_mesh_handle_get_next();
    GfxMesh* currentMesh = gfx_pool_gfx_mesh_get(outMeshHandle);
    currentMesh->numIndices = rawMesh.indexCount;
    currentMesh->numVertices = rawMesh.vertexCount;

    const uint32_t indexBufferSize = rawMesh.indexCount * sizeof(uint32_t);
    currentMesh->indexBuffer = gfx_buffer_create(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_AUTO);

    const uint32_t vertexBufferSize = rawMesh.vertexCount * sizeof(CoreVertex);
    currentMesh->vertexBuffer = gfx_buffer_create(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_AUTO);

    GfxBuffer staging = gfx_buffer_create(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO);
    {
        memcpy( (uint8_t*)staging.info.pMappedData, rawMesh.vertexData, vertexBufferSize);
        memcpy( (uint8_t*)staging.info.pMappedData + vertexBufferSize, rawMesh.indexData, indexBufferSize);

        VkCommandBuffer cmdBuffer = gfx_command_buffer_start_immediate_recording();
        {
            const VkBufferCopy vertexCopy = (VkBufferCopy){ .srcOffset = 0, .dstOffset = 0, .size = vertexBufferSize,};
            vkCmdCopyBuffer(cmdBuffer, staging.buffer, currentMesh->vertexBuffer.buffer, 1, &vertexCopy);

            const VkBufferCopy indexCopy = (VkBufferCopy){ .srcOffset = vertexBufferSize, .dstOffset = 0, .size = indexBufferSize};
            vkCmdCopyBuffer(cmdBuffer, staging.buffer, currentMesh->indexBuffer.buffer, 1, &indexCopy);
        }
        gfx_command_buffer_end_immediate_recording(cmdBuffer);
    }
    gfx_buffer_cleanup(&staging);
    return outMeshHandle;
}

GfxMeshHandle gfx_mesh_build_cube(){
     CoreVertex vertexData[24] = {
            //===POS================//===NORMAL=========//===UV======//===COLOUR=========
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{-0.5f, -0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{-0.5f, +0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{+0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, +0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f, +0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},

            {{+0.5f, +0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, +0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, +0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    };

    uint32_t indexData[36] = {
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7, 4,
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };

    const CoreRawMesh rawMesh = {
        .vertexData = vertexData,
        .indexData = indexData,
        .vertexCount = 24,
        .indexCount = 36,
    };

    return gfx_mesh_load_from_memory(rawMesh);
}

GfxMeshHandle gfx_mesh_build_quad(){
    CoreVertex vertexData[4] = {
            //===POS================//===NORMAL=========//===UV======//===COLOUR=========
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{+0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            {{-0.5f, +0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
    };

    uint32_t indexData[6] = {
        0, 1, 2, 2, 3, 0
    };

    const CoreRawMesh rawMesh = {
        .vertexData = &vertexData[0],
        .indexData = &indexData[0],
        .vertexCount = 4,
        .indexCount = 6,
    };

    return gfx_mesh_load_from_memory(rawMesh);
}

void gfx_mesh_cleanup(GfxMeshHandle handle){
    GfxMesh* currentMesh = gfx_pool_gfx_mesh_get(handle);
    {
        gfx_buffer_cleanup(&currentMesh->vertexBuffer);
        gfx_buffer_cleanup(&currentMesh->indexBuffer);
        *currentMesh = (GfxMesh){};
    }
    gfx_pool_gfx_mesh_release(handle);
}
//=============================================================================
