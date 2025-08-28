#ifndef GFX_VULKAN_INTERFACE_H
#define GFX_VULKAN_INTERFACE_H

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
//=============================================================================

//===API=======================================================================
VkInstance* gfx_vulkan_instance();
VkPhysicalDevice* gfx_vulkan_physical_device();
VkDevice* gfx_vulkan_device();
VkQueue* gfx_vulkan_queue();
VkAllocationCallbacks* gfx_vulkan_allocation_callbacks();
VkSampleCountFlagBits gfx_vulkan_sample_count();
GfxVulkanTargetAttachmentFormats* gfx_vulkan_target_attachment_formats();
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
