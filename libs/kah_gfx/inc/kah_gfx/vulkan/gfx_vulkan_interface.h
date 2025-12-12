#ifndef GFX_VULKAN_INTERFACE_H
#define GFX_VULKAN_INTERFACE_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
//=============================================================================

//===API=======================================================================
vec2u gfx_vulkan_swapchain_size();
GfxImage gfx_get_current_swapchain_image_data();
void gfx_flush();

uint32_t gfx_buffer_index();
uint32_t gfx_swap_chain_index();
uint32_t gfx_last_swap_chain_index();
VkPhysicalDeviceMemoryProperties gfx_get_device_memory_properties();
//=============================================================================

//===COMMAND_BUFFER============================================================
void gfx_command_buffer_begin_rendering(VkCommandBuffer cmdBuffer, const VkRenderingInfoKHR *renderingInfo);
void gfx_command_buffer_end_rendering(VkCommandBuffer cmdBuffer);
void gfx_command_buffer_insert_memory_barrier(VkCommandBuffer cmdBuffer, const VkImage *image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

VkCommandBuffer gfx_command_buffer_start_immediate_recording();
void gfx_command_buffer_end_immediate_recording(VkCommandBuffer cmdBuffer);
//=============================================================================

//===RENDER_TASKS==============================================================
void gfx_vulkan_clear_depth_run(VkCommandBuffer cmdBuffer, GfxRenderContext ctx);
void gfx_vulkan_lit_run(VkCommandBuffer cmdBuffer, GfxRenderContext ctx);
void gfx_vulkan_imgui_run(VkCommandBuffer cmdBuffer, GfxRenderContext ctx);
void gfx_vulkan_prepare_present_run(VkCommandBuffer cmdBuffer, GfxRenderContext ctx);
void gfx_vulkan_blit_image_to_swapchain_run(VkCommandBuffer cmdBuffer, GfxRenderContext ctx);
//=============================================================================

//===UTILS=====================================================================
VkSurfaceFormatKHR gfx_vulkan_utils_select_surface_format();
VkFormat gfx_vulkan_utils_find_depth_format(VkImageTiling desiredTilingFormat);
//=============================================================================


#endif //GFX_VULKAN_INTERFACE_H
