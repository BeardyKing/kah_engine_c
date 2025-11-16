#ifndef GFX_POOL_H
#define GFX_POOL_H

//===INCLUDES==================================================================
#include <stdint.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
//=============================================================================

//====DEFINES==================================================================
#define GFX_NULL_HANDLE UINT32_MAX
//=============================================================================

//===API=======================================================================
#define GFX_POOL_GFX_IMAGE_COUNT_MAX UINT16_MAX
GfxImageHandle  gfx_pool_get_gfx_image_handle();
GfxImage*       gfx_pool_get_gfx_image(GfxImageHandle handle);
void            gfx_pool_release_gfx_image(GfxImageHandle handle);

#define GFX_POOL_GFX_TEXTURE_COUNT_MAX UINT16_MAX
GfxTextureHandle    gfx_pool_get_gfx_texture_handle();
GfxTexture*         gfx_pool_get_gfx_texture(GfxTextureHandle handle);
void                gfx_pool_release_gfx_texture(GfxTextureHandle handle);

#define GFX_POOL_GFX_MESH_COUNT_MAX UINT16_MAX
GfxMeshHandle   gfx_pool_get_gfx_mesh_handle();
GfxMesh*        gfx_pool_get_gfx_mesh(GfxMeshHandle handle);
void            gfx_pool_release_gfx_mesh(GfxMeshHandle handle);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_pool_create();
void gfx_pool_cleanup();
//=============================================================================
#endif //GFX_POOL_H
