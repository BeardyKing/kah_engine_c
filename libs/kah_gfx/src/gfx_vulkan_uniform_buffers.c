//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_uniform_buffers.h>
#include <kah_gfx/gfx_types.h>

#include <kah_core/assert.h>
//=============================================================================

//===INTERNAL==================================================================
static struct UniformBufferObjects{
    GfxBuffer scene;
} s_uniformBufferObjects = {};
//=============================================================================

//===API=======================================================================
GfxBuffer* gfx_uniform_buffer_scene_get(){
    core_assert(s_uniformBufferObjects.scene.buffer != VK_NULL_HANDLE);
    return &s_uniformBufferObjects.scene;
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_uniform_buffers_create(){
    s_uniformBufferObjects.scene = gfx_buffer_create(
        sizeof(SceneUBO),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO
    );
#if KAH_DEBUG
    memset(s_uniformBufferObjects.scene.info.pMappedData, 0, sizeof(SceneUBO)); // Validate the memory was mapped by VMA.
#endif // KAH_DEBUG
}

void gfx_uniform_buffers_cleanup(){
    gfx_buffer_cleanup(&s_uniformBufferObjects.scene);
}
//=============================================================================
