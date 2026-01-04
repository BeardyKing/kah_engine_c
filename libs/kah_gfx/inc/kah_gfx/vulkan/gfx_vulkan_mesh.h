#ifndef GFX_VULKAN_MESH_H
#define GFX_VULKAN_MESH_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_types.h>

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
//=============================================================================

//===API=======================================================================
GfxMeshHandle gfx_mesh_load_from_memory(CoreRawMesh rawMesh);
void gfx_mesh_cleanup(GfxMeshHandle handle);
GfxMeshHandle gfx_mesh_build_quad();
GfxMeshHandle gfx_mesh_build_cube();
GfxMeshHandle gfx_mesh_build_octahedron();

//=============================================================================

#endif //GFX_VULKAN_MESH_H
