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
GfxImageHandle  gfx_pool_gfx_image_handle_get_next();
GfxImage*       gfx_pool_gfx_image_get(GfxImageHandle handle);
void            gfx_pool_gfx_image_release(GfxImageHandle handle);

#define GFX_POOL_GFX_TEXTURE_COUNT_MAX UINT16_MAX
GfxTextureHandle    gfx_pool_gfx_texture_handle_get_next();
GfxTexture*         gfx_pool_gfx_texture_get(GfxTextureHandle handle);
void                gfx_pool_gfx_texture_release(GfxTextureHandle handle);

#define GFX_POOL_GFX_MESH_COUNT_MAX UINT16_MAX
GfxMeshHandle   gfx_pool_gfx_mesh_handle_get_next();
GfxMesh*        gfx_pool_gfx_mesh_get(GfxMeshHandle handle);
void            gfx_pool_gfx_mesh_release(GfxMeshHandle handle);

#define GFX_POOL_TRANSFORM_COUNT_MAX UINT16_MAX
TransformHandle     gfx_pool_transform_handle_get_next();
Transform*          gfx_pool_transform_get(TransformHandle handle);
void                gfx_pool_transform_release(TransformHandle handle);

#define GFX_POOL_CAMERA_COUNT_MAX UINT16_MAX
CameraHandle    gfx_pool_camera_handle_get_next();
Camera*         gfx_pool_camera_get(CameraHandle handle);
void            gfx_pool_camera_release(CameraHandle handle);

#define GFX_POOL_CAMERA_ENT_COUNT_MAX UINT16_MAX
CameraEntityHandle  gfx_pool_camera_entity_handle_get_next();
CameraEntity*       gfx_pool_camera_entity_get(CameraEntityHandle handle);
void                gfx_pool_camera_entity_release(CameraEntityHandle handle);

#define GFX_POOL_LIT_ENT_COUNT_MAX UINT16_MAX
LitEntityHandle     gfx_pool_lit_entity_handle_get_next();
LitEntity*          gfx_pool_lit_entity_get(LitEntityHandle handle);
void                gfx_pool_lit_entity_release(LitEntityHandle handle);

#define GFX_POOL_LIT_MATERIAL_COUNT_MAX UINT16_MAX
LitMaterialHandle   gfx_pool_lit_material_handle_get_next();
LitMaterial*        gfx_pool_lit_material_get(LitMaterialHandle handle);
void                gfx_pool_lit_material_release(LitMaterialHandle handle);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_pool_create();
void gfx_pool_cleanup();
//=============================================================================
#endif //GFX_POOL_H
