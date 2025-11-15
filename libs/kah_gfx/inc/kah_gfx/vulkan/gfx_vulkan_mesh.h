#ifndef GFX_VULKAN_MESH_H
#define GFX_VULKAN_MESH_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_buffer.h>

#include <kah_math/vec3.h>
#include <kah_math/vec2.h>
//=============================================================================

//===PUBLIC_TYPES==============================================================
struct CoreVertex {
    vec3f pos;
    vec3f normal;
    vec2f uv;
    vec3f color;
}typedef CoreVertex;

struct CoreRawMesh {
    CoreVertex *vertexData;
    uint32_t *indexData;

    uint32_t vertexCount;
    uint32_t indexCount;
} typedef CoreRawMesh;

struct GfxMesh{
    GfxBuffer vertexBuffer;
    GfxBuffer indexBuffer;
    uint32_t numVertices;
    uint32_t numIndices;
} typedef GfxMesh;
//=============================================================================

//===API=======================================================================
GfxMesh gfx_mesh_load_from_memory(CoreRawMesh rawMesh);
void gfx_mesh_cleanup(GfxMesh* inMesh);
GfxMesh gfx_mesh_build_plane();
//=============================================================================

#endif //GFX_VULKAN_MESH_H
