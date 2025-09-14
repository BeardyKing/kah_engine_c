
//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan_resource.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>

#include <kah_core/assert.h>
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===INTERNAL==================================================================
static void gfx_image_free(GfxImage* image){
    vmaDestroyImage(g_gfx.allocator, image->image, image->alloc);
    vkDestroyImageView(g_gfx.device, image->view, g_gfx.allocationCallbacks);
}
//=============================================================================

//===API=======================================================================
GfxImageHandle gfx_resource_image_depth_create(GfxAttachmentInfo* info){
    const VkFormat targetFormat = info->format == VK_FORMAT_UNDEFINED ? gfx_vulkan_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL) : info->format;
    const vec2u targetSize = info->sizeType == GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE ? gfx_vulkan_swapchain_size() : info->size;

    const VkImageCreateInfo depthImageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = targetFormat,
            .extent = {
                .width = targetSize.x,
                .height = targetSize.y,
                .depth =  1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = g_gfx.sampleCount,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    };

    VmaAllocationCreateInfo allocInfo = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT,
    };

    GfxImageHandle outHandle = gfx_pool_get_gfx_image_handle();
    core_assert(outHandle != GFX_NULL_HANDLE);
    GfxImage* currentImage = gfx_pool_get_gfx_image(outHandle);

    const VkResult createImgRes = vmaCreateImage(g_gfx.allocator, &depthImageInfo, &allocInfo, &currentImage->image, &currentImage->alloc, nullptr);
    core_assert(createImgRes == VK_SUCCESS);

    VkResult bindRes = vmaBindImageMemory( g_gfx.allocator, currentImage->alloc, currentImage->image );
    core_assert_msg(bindRes == VK_SUCCESS, "err: Failed to bind depth stencil buffer");

    VkImageViewCreateInfo depthImageViewInfo = (VkImageViewCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = currentImage->image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = targetFormat,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    if (targetFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        depthImageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    const VkResult createRes = vkCreateImageView(g_gfx.device, &depthImageViewInfo, nullptr, &currentImage->view);
    core_assert_msg(createRes == VK_SUCCESS, "err: Failed to create depth image view");
    return outHandle;

}

GfxImageHandle gfx_resource_image_colour_create(GfxAttachmentInfo* info){
    const VkFormat targetFormat = info->format == VK_FORMAT_UNDEFINED ? gfx_vulkan_utils_select_surface_format().format : info->format;
    const vec2u targetSize = info->sizeType == GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE ? gfx_vulkan_swapchain_size() : info->size;

    VkImageCreateInfo colourImageInfo = (VkImageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = targetFormat,
        .extent = (VkExtent3D){
            .width = targetSize.x,
            .height = targetSize.y,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = g_gfx.sampleCount,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocInfo = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT
    };

    GfxImageHandle outHandle = gfx_pool_get_gfx_image_handle();
    core_assert(outHandle != GFX_NULL_HANDLE);
    GfxImage* currentImage = gfx_pool_get_gfx_image(outHandle);
    currentImage->size = targetSize;

    const VkResult createImgRes = vmaCreateImage(g_gfx.allocator, &colourImageInfo, &allocInfo, &currentImage->image, &currentImage->alloc, nullptr);
    core_assert(createImgRes == VK_SUCCESS);

    VkResult bindRes = vmaBindImageMemory( g_gfx.allocator, currentImage->alloc, currentImage->image );
    core_assert_msg(bindRes == VK_SUCCESS, "err: Failed to bind color buffer");

    VkImageViewCreateInfo colorImageViewInfo = (VkImageViewCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = currentImage->image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = targetFormat,
        .subresourceRange = (VkImageSubresourceRange){
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    const VkResult createRes = vkCreateImageView(g_gfx.device, &colorImageViewInfo, g_gfx.allocationCallbacks, &currentImage->view);
    core_assert_msg(createRes == VK_SUCCESS, "err: Failed to create color image view");
    return outHandle;
}


uint32_t gfx_resource_create_type(GfxResourceType type, GfxAttachmentInfo* info){
    if(type == GFX_RESOURCE_IMAGE_COLOR){
        return gfx_resource_image_colour_create(info);
    }
    if (type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
        return gfx_resource_image_depth_create(info);
    }
    core_not_implemented();
    return UINT32_MAX;
}

void gfx_resource_image_release(GfxImageHandle gfxHandle){
    gfx_image_free(gfx_pool_get_gfx_image(gfxHandle));
}
//=============================================================================
