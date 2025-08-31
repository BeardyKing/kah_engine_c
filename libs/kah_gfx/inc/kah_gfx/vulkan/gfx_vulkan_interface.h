#ifndef GFX_VULKAN_INTERFACE_H
#define GFX_VULKAN_INTERFACE_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
//=============================================================================

//===API=======================================================================
vec2u gfx_vulkan_swapchain_size();
//=============================================================================

//===RENDER_TASKS==============================================================
void gfx_vulkan_clear_depth_run(VkCommandBuffer cmdBuffer);
void gfx_vulkan_imgui_run(VkCommandBuffer cmdBuffer);
void gfx_vulkan_prepare_present_run(VkCommandBuffer cmdBuffer);
//=============================================================================

//===UTILS=====================================================================
VkSurfaceFormatKHR gfx_vulkan_utils_select_surface_format();
VkFormat gfx_vulkan_utils_find_depth_format(VkImageTiling desiredTilingFormat);
//=============================================================================

#endif //GFX_VULKAN_INTERFACE_H
