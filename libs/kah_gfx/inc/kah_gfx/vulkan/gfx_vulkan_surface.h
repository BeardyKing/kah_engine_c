#ifndef GFX_VULKAN_SURFACE_H
#define GFX_VULKAN_SURFACE_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===API=======================================================================
void gfx_create_surface(void *windowHandle, const VkInstance *instance, VkSurfaceKHR *outSurface);
//=============================================================================

#endif //GFX_VULKAN_SURFACE_H
