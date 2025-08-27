#ifndef GFX_VULKAN_IMGUI_H
#define GFX_VULKAN_IMGUI_H

#include <kah_core/defines.h>
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
//=============================================================================

//===API=======================================================================
void gfx_imgui_begin();
void gfx_imgui_end();
void gfx_imgui_draw(VkCommandBuffer cmdBuffer);

void gfx_imgui_demo_window();
void *gfx_imgui_get_proc_function_pointer();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_imgui_create(void *windowHandle);
void gfx_imgui_cleanup();
//=============================================================================
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)

#endif //GFX_VULKAN_IMGUI_H
