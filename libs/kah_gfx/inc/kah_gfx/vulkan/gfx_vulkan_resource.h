#ifndef GFX_VULKAN_RESOUCE_H
#define GFX_VULKAN_RESOUCE_H

//===INCLUDES==================================================================
#include <kah_gfx/gfx_pool.h>
//=============================================================================

//===API=======================================================================
GfxImageHandle gfx_resource_image_depth_create(GfxAttachmentInfo* info);
void gfx_resource_image_release(GfxImageHandle gfxHandle);
//=============================================================================

#endif //GFX_VULKAN_RESOUCE_H
