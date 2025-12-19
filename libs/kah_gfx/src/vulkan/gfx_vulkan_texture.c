#include <kah_gfx/vulkan/gfx_vulkan_texture.h>
#include <kah_gfx/vulkan/gfx_vulkan_utils.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_buffer.h>
#include <kah_gfx/gfx_pool.h>
#include <kah_gfx/gfx_converter.h>

#include <kah_core/texture_formats.h>
#include <kah_core/dds_loader.h>
#include <kah_core/assert.h>
#include <kah_core/memory.h>
#include <kah_core/c_string.h>
#include <kah_core/defines.h>
#include <kah_core/filesystem.h>

#include <string.h>

//===INTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===INTERNAL==================================================================
static void set_image_layout(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask) {
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier = (VkImageMemoryBarrier){
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .oldLayout = oldImageLayout,
        .newLayout = newImageLayout,
        .image = image,
        .subresourceRange = subresourceRange,
    };

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // Image layout is undefined (or does not matter)
            // Only valid as initial layout
            // No flags required, listed only for completeness
            imageMemoryBarrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            // Image is preinitialized
            // Only valid as initial layout for linear images, preserves memory contents
            // Make sure host writes have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image is a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image is a depth/stencil attachment
            // Make sure any writes to the depth/stencil buffer have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image is a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image is a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image is read by a shader
            // Make sure any shader reads from the image have been finished
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used as a transfer destination
            // Make sure any writes to the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used as a transfer source
            // Make sure any reads from the image have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment
            // Make sure any writes to the color buffer have been finished
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image layout will be used as a depth/stencil attachment
            // Make sure any writes to depth/stencil buffer have been finished
            imageMemoryBarrier.dstAccessMask =
                    imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read in a shader (sampler, input attachment)
            // Make sure any writes to the image have been finished
            if (imageMemoryBarrier.srcAccessMask == 0) {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
            cmdbuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
}

static GfxTextureHandle gfx_texture_create( const VkImageCreateInfo createInfo) {
    const VmaAllocationCreateInfo allocInfo = (VmaAllocationCreateInfo){
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };

    const GfxTextureHandle outTextureHandle = gfx_pool_gfx_texture_handle_get_next();
    GfxTexture* currentTexture = gfx_pool_gfx_texture_get(outTextureHandle);

    currentTexture->format = createInfo.format;
    currentTexture->usage = createInfo.usage;
    currentTexture->extent = createInfo.extent;
    currentTexture->mipLevels = createInfo.mipLevels;
    currentTexture->arrayLayers = createInfo.arrayLayers;

    const VkResult imageRes = vmaCreateImage(g_gfx.allocator, &createInfo, &allocInfo, &currentTexture->image, &currentTexture->allocation, nullptr);
    core_assert(imageRes == VK_SUCCESS);

    const bool shouldCreateView = ((createInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0) || ((createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) ||((createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0) || ((createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0);

    if (shouldCreateView) {
        VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
        if (createInfo.format == VK_FORMAT_D32_SFLOAT) { // TODO: support other depth formats
            aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        const uint32_t viewType = createInfo.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;

        const VkImageViewCreateInfo viewCreateInfo = (VkImageViewCreateInfo){
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = currentTexture->image,
            .viewType = viewType,
            .format = createInfo.format,
            .subresourceRange =
                (VkImageSubresourceRange){
                    .aspectMask = aspectFlag,
                    .baseMipLevel = 0,
                    .levelCount = currentTexture->mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = createInfo.arrayLayers,
                },
        };

        const VkResult viewRes = vkCreateImageView(g_gfx.device, &viewCreateInfo, g_gfx.allocationCallbacks, &currentTexture->imageView);
        core_assert(viewRes == VK_SUCCESS);
    }

    return outTextureHandle;
}
//=============================================================================

//===API=======================================================================
GfxTextureHandle gfx_texture_load_from_file( const char* path ){
    CoreRawImage rawImage = (CoreRawImage){};

    //TODO: read EXT and based on that select different load methods. currently only DDS is supported.
    const char* ext = c_str_search_reverse((char*)path, ".");
    core_assert(c_str_search_reverse((char*)ext, ".") != nullptr);

    if(c_str_equal(ext, ".dds")){
#if CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)
        if (!fs_file_is_absolute(path)){
            gfx_convert_texture_dds((char*)path);
        }
#endif //CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)
        load_dds_image_alloc(allocators()->cstd, path, &rawImage);
    }
    core_assert_msg(rawImage.imageData->bufferAddress, "err: failed to load file.");
    //TODO: replace code below with a new func called gfx_texture_load_from_memory().
    //TODO: caching here. (only read header info & based on that decide if the load should continue.
    GfxBuffer uploadBuffer = gfx_buffer_create(rawImage.dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO);
    memcpy(uploadBuffer.info.pMappedData, rawImage.imageData->bufferAddress, rawImage.dataSize);

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = gfx_utils_core_image_format_to_vk(rawImage.textureFormat),
        .extent = (VkExtent3D){
            .width = rawImage.width,
            .height = rawImage.height,
            .depth = 1,
        },
        .mipLevels = rawImage.mipMapCount,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = 0,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    GfxTextureHandle outTextureHandle = gfx_texture_create(imageCreateInfo);
    GfxTexture* currentTexture = gfx_pool_gfx_texture_get(outTextureHandle);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = currentTexture->mipLevels;
    subresourceRange.layerCount = 1;

    AllocInfo* bufferCopyRegionsAlloc = mem_cstd_alloc(currentTexture->mipLevels * sizeof(VkBufferImageCopy));
    {
        VkBufferImageCopy *bufferCopyRegions = (VkBufferImageCopy *) bufferCopyRegionsAlloc->bufferAddress;
        size_t offset = 0;
        for (uint32_t i = 0; i < currentTexture->mipLevels; i++) {
            // set up a buffer image copy structure for the current mip level
            bufferCopyRegions[i] = (VkBufferImageCopy){
                .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .imageSubresource.mipLevel = i,
                .imageSubresource.baseArrayLayer = 0,
                .imageSubresource.layerCount = 1,
                .imageExtent.width = currentTexture->extent.width >> i,
                .imageExtent.height = currentTexture->extent.height >> i,
                .imageExtent.depth = 1,
                .bufferOffset = offset,
            };
            offset += rawImage.mipDataSizes[i];
        }

        VkCommandBuffer cmdBuffer = gfx_command_buffer_start_immediate_recording();
        {
            set_image_layout(
                    cmdBuffer,
                    currentTexture->image,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    subresourceRange,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
            );

            // Copy mips from staging buffer
            vkCmdCopyBufferToImage(
                    cmdBuffer,
                    uploadBuffer.buffer,
                    currentTexture->image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    currentTexture->mipLevels,
                    bufferCopyRegions
            );

            set_image_layout(
                    cmdBuffer,
                    currentTexture->image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    subresourceRange,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
            );
        }
        gfx_command_buffer_end_immediate_recording(cmdBuffer);
    }
    mem_cstd_free(bufferCopyRegionsAlloc);
    allocators()->cstd.free(rawImage.imageData);
    gfx_buffer_cleanup(&uploadBuffer);
    return outTextureHandle;
}

void gfx_texture_cleanup(GfxTextureHandle handle) {
    GfxTexture* currentTexture = gfx_pool_gfx_texture_get(handle);
    {
        vkDestroyImageView(g_gfx.device, currentTexture->imageView, g_gfx.allocationCallbacks);
        vmaDestroyImage(g_gfx.allocator, currentTexture->image, currentTexture->allocation);
        *currentTexture = (GfxTexture){};
    }
    gfx_pool_gfx_texture_release(handle);
}
//=============================================================================