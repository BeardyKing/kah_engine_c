#ifndef GFX_VULKAN_INTERFACE_H
#define GFX_VULKAN_INTERFACE_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
//=============================================================================

//===API=======================================================================
vec2u gfx_vulkan_swapchain_size();
GfxImage gfx_get_current_swapchain_image_data();
//=============================================================================

//===COMMAND_BUFFER============================================================
void gfx_command_begin_rendering(VkCommandBuffer cmdBuffer, const VkRenderingInfoKHR *renderingInfo);
void gfx_command_end_rendering(VkCommandBuffer cmdBuffer);
//=============================================================================

//===ATTACHMENTS===============================================================
VkRenderingAttachmentInfoKHR gfx_rendering_attachment_info_depth_spencil(VkImageView imageView);
VkRenderingAttachmentInfoKHR gfx_rendering_attachment_info_color(VkImageView imageView);
//=============================================================================

//===RENDER_TASKS==============================================================
void gfx_vulkan_clear_depth_run(VkCommandBuffer cmdBuffer, VkRenderingInfoKHR renderingInfo, GfxRenderContext ctx);
void gfx_vulkan_imgui_run(VkCommandBuffer cmdBuffer, VkRenderingInfoKHR renderingInfo, GfxRenderContext ctx);
void gfx_vulkan_prepare_present_run(VkCommandBuffer cmdBuffer, VkRenderingInfoKHR renderingInfo, GfxRenderContext ctx);
void gfx_vulkan_blit_image_to_swapchain_run(VkCommandBuffer cmdBuffer, VkRenderingInfoKHR renderingInfo, GfxRenderContext ctx);
//=============================================================================

//===UTILS=====================================================================
VkSurfaceFormatKHR gfx_vulkan_utils_select_surface_format();
VkFormat gfx_vulkan_utils_find_depth_format(VkImageTiling desiredTilingFormat);
//=============================================================================

#endif //GFX_VULKAN_INTERFACE_H
