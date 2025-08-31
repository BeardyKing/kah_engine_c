#ifndef GFX_POOL_H
#define GFX_POOL_H

//===INCLUDES==================================================================
#include <stdint.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
//=============================================================================

//===API=======================================================================
typedef uint32_t GfxImageHandle;
GfxImageHandle  gfx_pool_get_gfx_image_handle();
GfxImage*       gfx_pool_get_gfx_image(GfxImageHandle);
void            gfx_pool_release_gfx_image(GfxImageHandle);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_pool_create();
void gfx_pool_cleanup();
//=============================================================================
#endif //GFX_POOL_H
