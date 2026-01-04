#ifndef GFX_VULKAN_TEXTURE_H
#define GFX_VULKAN_TEXTURE_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
//=============================================================================

//===API=======================================================================
GfxTextureHandle gfx_texture_load_from_file( const char* path );
void gfx_texture_cleanup(GfxTextureHandle handle);
//=============================================================================

//===DEFINES===================================================================
#define KAH_BINDLESS_TEXTURE_BLACK 0
#define KAH_BINDLESS_TEXTURE_WHITE 1
#define KAH_BINDLESS_TEXTURE_UV 2
#define KAH_BINDLESS_TEXTURE_UV_OCTAHEDRAL 3
//=============================================================================

#endif //GFX_VULKAN_TEXTURE_H
