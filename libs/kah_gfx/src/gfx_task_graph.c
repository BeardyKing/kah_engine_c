//===INCLUDES==================================================================
#include <kah_gfx/gfx_task_graph.h>
#include <kah_gfx/gfx_logging.h>
#include <kah_gfx/gfx_pool.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_resource.h>
#include <kah_gfx/vulkan/gfx_vulkan_utils.h>

#include <kah_math/utils.h>

#include <kah_core/allocators.h>
#include <kah_core/assert.h>
#include <kah_core/utils.h>

#include <stdio.h>
//=============================================================================

//===LOCAL_STRUCTS=============================================================
constexpr size_t TG_RENDER_PASS_MAX = 128;
[[maybe_unused]] constexpr size_t TG_ATTACHMENTS_MAX = 128;
constexpr size_t TG_RENDER_PASS_NAME_MAX = 64;
constexpr size_t TG_RENDER_PASS_ATTACHMENT_MAX = 8;
constexpr char TG_ARENA_STRING_FALLBACK[TG_RENDER_PASS_NAME_MAX] = "err: no tg string memory remaining";
constexpr bool TG_PRINT_INFO = false;

struct GfxTaskGraph typedef GfxTaskGraph;
struct GfxRenderPass typedef GfxRenderPass;

typedef void (*GfxRenderPassRun_cb)(VkCommandBuffer cmd, GfxRenderContext ctx);
typedef uint32_t GfxResourceInfoHandle;

enum GfxRenderGraphQueueType {
    GFX_RENDER_GRAPH_QUEUE_NONE = 0,
    GFX_RENDER_GRAPH_QUEUE_GRAPHICS = 1 << 0,
    GFX_RENDER_GRAPH_QUEUE_BLIT = 1 << 1,
    GFX_RENDER_GRAPH_QUEUE_COMPUTE = 1 << 2,
    GFX_RENDER_GRAPH_QUEUE_PRESENT = 1 << 3,
}typedef GfxRenderGraphQueueType;

enum GfxAttachmentSizeType{
    ATTACHMENT_NONE,
    ATTACHMENT_ABSOLUTE,
    ATTACHMENT_SWAPCHAIN
}typedef GfxAttachmentSizeType;

struct GfxResourceInfo{
    const char* name;
    uint32_t resourceHandle;

    GfxAttachmentInfo attachmentInfo;

    VkImageLayout lastLayout;
    VkAccessFlagBits lastAccess;
    VkPipelineStageFlagBits lastStage;
} typedef GfxResourceInfo;

struct PassCtx{
    uint32_t binding;
    GfxResourceInfoHandle handle;

    VkAccessFlagBits acess;
    VkImageLayout layout;
    VkPipelineStageFlagBits stage;
    bool clear;
}typedef PassCtx;

struct GfxRenderInfoContext{
    PassCtx read[GFX_RENDER_CONTEXT_READ_WRITE_MAX];
    uint32_t readCount;

    PassCtx write[GFX_RENDER_CONTEXT_READ_WRITE_MAX];
    uint32_t writeCount;
}typedef GfxRenderInfoContext;

struct GfxRenderPass {
    GfxRenderGraphQueueType queue;
    const char* name;
    GfxRenderPassRun_cb dispatch;

    GfxRenderInfoContext infoCtx;

    GfxAttachmentSizeType sizeType;
    vec2u attachmentSize;
}typedef GfxRenderPass;

struct GfxTaskGraph {
    GfxRenderPass renderPasses[TG_RENDER_PASS_MAX];
    uint32_t renderPassCount;
} typedef GfxTaskGraph;
//=============================================================================

//===LOCAL_VARIABLES===========================================================
static GfxTaskGraph s_tg = {};

static struct TaskGraphResourceInfoHandles{
    GfxResourceInfoHandle color;
    GfxResourceInfoHandle depthStencil;
    GfxResourceInfoHandle external_swapchainImage[KAH_SWAP_CHAIN_IMAGE_COUNT];
} s_resources;

struct TaskGraphArena{
    AllocInfo* alloc;
    size_t count;
} typedef TaskGraphArena;

static TaskGraphArena s_tgRenderPassArena = {};
static TaskGraphArena s_tgResourceArena = {};
static bool s_tgArenasAllocated = false;

constexpr uint32_t TG_RESOURCE_POOL_MAX = 32;
static struct GfxTaskGraphPools{
    GfxResource gfxResources[TG_RESOURCE_POOL_MAX];
    uint32_t gfxResourcesCount;

    GfxResourceInfo gfxResourceInfos[TG_RESOURCE_POOL_MAX];
    uint32_t gfxResourceInfosCount;
}s_tgPools = {};
//=============================================================================

//===LOCAL_FUNCTIONS===========================================================
#define task_graph_log(fmt, ...) if (TG_PRINT_INFO) { gfx_log_info(fmt, ##__VA_ARGS__); }

static const char* task_graph_arena_string(TaskGraphArena* inArena, const char* inStr){
    core_assert(inArena->alloc);
    const size_t inStrLen = strlen(inStr) + 1;
    if((inArena->count + inStrLen) < inArena->alloc->commitedMemory){
        core_assert(inStrLen < TG_RENDER_PASS_NAME_MAX);
        char* outStr = &((char*)inArena->alloc->bufferAddress)[s_tgRenderPassArena.count];
        inArena->count += inStrLen;
        sprintf(outStr, "%s\n", inStr);
        return outStr;
    }
    return TG_ARENA_STRING_FALLBACK;
}

static GfxRenderPass* render_pass_create(const char* renderPassName){
    core_assert(s_tg.renderPassCount + 1 < TG_RENDER_PASS_MAX);
    GfxRenderPass* outRP = &s_tg.renderPasses[s_tg.renderPassCount++];
    outRP->name = task_graph_arena_string(&s_tgRenderPassArena, renderPassName);
    return outRP;
}

static VkRenderingAttachmentInfoKHR gfx_rendering_attachment_info_depth_stencil(VkImageView imageView, bool shouldClear){
    return (VkRenderingAttachmentInfoKHR){
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .loadOp = shouldClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.depthStencil = {.depth = 1.0f, .stencil = 0}}, //TODO: Remove.
    };
}

static VkRenderingAttachmentInfoKHR gfx_rendering_attachment_info_color(VkImageView imageView, bool shouldClear){
    return (VkRenderingAttachmentInfoKHR){
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = shouldClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.color = {{0.5f, 0.092f, 0.167f, 1.0f}},},
    };
}

GfxResourceInfoHandle gfx_task_graph_resource_create(GfxResourceType type, GfxAttachmentInfo* attachmentInfo, const char* resourceName, GfxImageExternal_cb cb){
    core_assert(s_tgPools.gfxResourcesCount + 1 < TG_RESOURCE_POOL_MAX);
    core_assert(s_tgPools.gfxResourceInfosCount + 1 < TG_RESOURCE_POOL_MAX);
    const uint32_t outResourcePoolIndex = s_tgPools.gfxResourcesCount;
    const uint32_t outResourceInfoIndex = s_tgPools.gfxResourceInfosCount;
    core_assert_msg(outResourcePoolIndex == outResourceInfoIndex, "resource & pool must be 1:1 for fast info lookup");
    {
        GfxResource* resource = &s_tgPools.gfxResources[s_tgPools.gfxResourcesCount++];

        if(type == GFX_RESOURCE_IMAGE_COLOR){
            const uint32_t resourceDataHandle = gfx_resource_create_type(type, attachmentInfo);
            *resource = (GfxResource){.type = type, .data.imageColor = {.binding = TG_INVALID, .handle = resourceDataHandle}};
        }
        if(type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            const uint32_t resourceDataHandle = gfx_resource_create_type(type, attachmentInfo);
            *resource = (GfxResource){.type = type, .data.imageDepthStencil = {.handle = resourceDataHandle}};
        }
        if(type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
            core_assert(cb != nullptr);
            *resource = (GfxResource){.type = type, .data.external = {.binding = TG_INVALID, .imageCB = cb}};
        }
    }
    {
        const char* resName = task_graph_arena_string(&s_tgResourceArena, resourceName);
        s_tgPools.gfxResourceInfos[s_tgPools.gfxResourceInfosCount++] = (GfxResourceInfo){
            .name = resName,
            .resourceHandle = outResourcePoolIndex,
            .attachmentInfo = attachmentInfo != nullptr ? *attachmentInfo : (GfxAttachmentInfo){},
            .lastLayout = 0,
            .lastAccess = 0, // not a valid enum entry.
            .lastStage = 0,
        };
    }
    return outResourceInfoIndex;
}

static void gfx_task_graph_build_and_run_barriers(VkCommandBuffer cmdBuffer, const PassCtx* passCtxArray, const uint32_t passCount){
    for (uint32_t readIndex = 0; readIndex < passCount; ++readIndex){
        const PassCtx* ctx = &passCtxArray[readIndex];
        const GfxResource* resource = &s_tgPools.gfxResources[ctx->handle];
        GfxResourceInfo* info = &s_tgPools.gfxResourceInfos[ctx->handle];

        const bool accessMatch = info->lastAccess == ctx->acess;
        const bool layoutMatch = info->lastLayout == ctx->layout;
        const bool stageMatch = info->lastStage == ctx->stage;


        if(accessMatch && layoutMatch && stageMatch){
            continue;
        }

        if(resource->type != GFX_RESOURCE_NONE){
            VkImageAspectFlags imageAspectFlags = {};
            GfxImage gfxImage = {};
            if(resource->type == GFX_RESOURCE_IMAGE_COLOR){
                gfxImage = *gfx_pool_get_gfx_image(resource->data.imageColor.handle);
                imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            if(resource->type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
                gfxImage = *gfx_pool_get_gfx_image(resource->data.imageDepthStencil.handle);
                imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            if(resource->type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
                gfxImage = resource->data.external.imageCB();
                imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            core_assert(gfxImage.image != VK_NULL_HANDLE);

            task_graph_log("tg: Barrier before: \noldLayout = \t%s \nsrcAccessMask = %s \nsrcStageMask = \t%s\n", VkImageLayout_c_str(info->lastLayout), VkAccessFlagBits_c_str(info->lastAccess) , VkPipelineStageFlags_c_str(info->lastStage));
            task_graph_log("tg: Barrier after : \nnewLayout = \t%s \ndstAccessMask = %s \ndstStageMask = \t%s\n\n", VkImageLayout_c_str(ctx->layout), VkAccessFlagBits_c_str(ctx->acess) , VkPipelineStageFlags_c_str(ctx->stage));

            gfx_command_buffer_insert_memory_barrier(
                cmdBuffer,
                &gfxImage.image,
                info->lastAccess,
                ctx->acess,
                info->lastLayout,
                ctx->layout,
                info->lastStage,
                ctx->stage,
                (VkImageSubresourceRange){imageAspectFlags, 0, 1, 0, 1}
            );

            info->lastAccess = ctx->acess;
            info->lastLayout = ctx->layout;
            info->lastStage = ctx->stage;
        }
    }
}

static void build_render_context_from_info_context(GfxRenderContext* ctx, const GfxRenderInfoContext* info){
    core_assert(ctx != nullptr);
    core_assert(info != nullptr);

    ctx->readCount = info->readCount;
    ctx->writeCount = info->writeCount;

    for (uint32_t readIndex = 0; readIndex < info->readCount; ++readIndex){
        const uint32_t bindingIndex = info->read[readIndex].binding;
        ctx->read[readIndex] = s_tgPools.gfxResources[info->read[readIndex].handle];
        ctx->read[readIndex].clear = info->read[readIndex].clear;

        if(ctx->read[readIndex].type == GFX_RESOURCE_IMAGE_COLOR ){
            ctx->read[readIndex].data.imageColor.binding = bindingIndex;
        }
        if(ctx->read[readIndex].type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            core_assert(bindingIndex == TG_INVALID);
        }
        if(ctx->read[readIndex].type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
            ctx->read[readIndex].data.external.binding = bindingIndex;
        }
    }

    for (uint32_t writeIndex = 0; writeIndex < info->writeCount; ++writeIndex){
        const uint32_t bindingIndex = info->write[writeIndex].binding;
        ctx->write[writeIndex] = s_tgPools.gfxResources[info->write[writeIndex].handle];
        ctx->write[writeIndex].clear = info->write[writeIndex].clear;

        if(ctx->write[writeIndex].type == GFX_RESOURCE_IMAGE_COLOR ){
            ctx->write[writeIndex].data.imageColor.binding = bindingIndex;
        }
        if(ctx->write[writeIndex].type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            core_assert(bindingIndex == TG_INVALID);
        }
        if(ctx->write[writeIndex].type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
            ctx->write[writeIndex].data.external.binding = bindingIndex;
        }
    }
}

static void gfx_task_graph_run_present(VkCommandBuffer cmdBuffer, const GfxRenderPass* rp){
    core_assert(rp->infoCtx.readCount == 1 && rp->infoCtx.writeCount == 0);
    task_graph_log("render pass present: %s\n", rp->name);
    GfxRenderContext renderCtx = {};
    build_render_context_from_info_context(&renderCtx, &rp->infoCtx);

    rp->dispatch(cmdBuffer, renderCtx);
}

static void gfx_task_graph_run_blit(VkCommandBuffer cmdBuffer, const GfxRenderPass* rp){
    core_assert(rp->infoCtx.readCount == 1 && rp->infoCtx.writeCount == 1);
    task_graph_log("render pass blit: %s\n", rp->name);

    GfxRenderContext renderCtx = {};
    build_render_context_from_info_context(&renderCtx, &rp->infoCtx);

    rp->dispatch(cmdBuffer, renderCtx);
}

static void gfx_task_graph_run_graphics(VkCommandBuffer cmdBuffer, const GfxRenderPass* rp){
    task_graph_log("render pass graphics: %s\n", rp->name);

    core_assert(rp->sizeType != ATTACHMENT_NONE || (rp->attachmentSize.x == 0 && rp->attachmentSize.y == 0));
    core_assert(rp->sizeType != ATTACHMENT_ABSOLUTE || (rp->attachmentSize.x != 0 && rp->attachmentSize.y != 0));
    const vec2u size = rp->sizeType == ATTACHMENT_SWAPCHAIN ? gfx_vulkan_swapchain_size() : rp->attachmentSize;

    VkRenderingAttachmentInfoKHR depthStencilAttachment = {};
    bool depthStencilAttachmentInUse = false;

    VkRenderingAttachmentInfoKHR colorAttachments[TG_RENDER_PASS_ATTACHMENT_MAX] = {};
    bool colorAttachmentsInUse[TG_RENDER_PASS_ATTACHMENT_MAX] = {};
    uint32_t colorAttachmentCount = 0;

    GfxRenderContext renderCtx = {};
    build_render_context_from_info_context(&renderCtx, &rp->infoCtx);

    for (uint32_t writeIndex = 0; writeIndex < renderCtx.writeCount; ++writeIndex){
        GfxResourceType type = renderCtx.write[writeIndex].type;
        const bool shouldClear = renderCtx.write[writeIndex].clear;

        if(type == GFX_RESOURCE_IMAGE_COLOR){
            const uint32_t binding = renderCtx.write[writeIndex].data.imageColor.binding;
            core_assert(colorAttachmentsInUse[binding] == false);
            colorAttachments[binding] = gfx_rendering_attachment_info_color(gfx_pool_get_gfx_image(renderCtx.write[writeIndex].data.imageColor.handle)->view, shouldClear);
            colorAttachmentsInUse[binding] = true;
            colorAttachmentCount = max_i32(colorAttachmentCount, binding + 1);
        }
        if(type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            core_assert(depthStencilAttachmentInUse == false);
            depthStencilAttachment = gfx_rendering_attachment_info_depth_stencil(gfx_pool_get_gfx_image(renderCtx.write[writeIndex].data.imageDepthStencil.handle)->view, shouldClear);
            depthStencilAttachmentInUse = true;
        }
        if(type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
            // Note: we probably want (some) external resources to be included in the color attachment count,
            // but we don't sore that info currently. (i.e. not all external resources are colour)
        }
    }

    const VkRenderingInfoKHR renderingInfo = (VkRenderingInfoKHR){
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .renderArea = {.offset = {0, 0}, .extent = {size.x, size.y}},
        .layerCount = 1,
        .colorAttachmentCount = colorAttachmentCount,
        .pColorAttachments = colorAttachmentCount ? colorAttachments : nullptr,
        .pDepthAttachment = depthStencilAttachmentInUse ? &depthStencilAttachment : nullptr,
        .pStencilAttachment = depthStencilAttachmentInUse ? &depthStencilAttachment : nullptr,
    };

    gfx_command_buffer_begin_rendering(cmdBuffer, &renderingInfo);
    {
        rp->dispatch(cmdBuffer, renderCtx);
    }
    gfx_command_buffer_end_rendering(cmdBuffer);
}
//=============================================================================

//===API=======================================================================
void gfx_task_graph_run(VkCommandBuffer cmdBuffer){
    task_graph_log("tg begin:\n");
    for (uint32_t renderPassIndex = 0; renderPassIndex < s_tg.renderPassCount; ++renderPassIndex){
        const GfxRenderPass* rp = &s_tg.renderPasses[renderPassIndex];
        core_assert(rp->infoCtx.readCount <= GFX_RENDER_CONTEXT_READ_WRITE_MAX);
        core_assert(rp->infoCtx.writeCount <= GFX_RENDER_CONTEXT_READ_WRITE_MAX);

        //TODO: gfx_task_graph_build_and_run_barriers should batch barriers & skip when last == current resource state.
        gfx_task_graph_build_and_run_barriers(cmdBuffer,rp->infoCtx.read, rp->infoCtx.readCount);
        gfx_task_graph_build_and_run_barriers(cmdBuffer, rp->infoCtx.write, rp->infoCtx.writeCount);

        switch (rp->queue){
        case GFX_RENDER_GRAPH_QUEUE_GRAPHICS:
            gfx_task_graph_run_graphics(cmdBuffer, rp);
            break;
        case GFX_RENDER_GRAPH_QUEUE_BLIT:
            gfx_task_graph_run_blit(cmdBuffer, rp);
            break;
        case GFX_RENDER_GRAPH_QUEUE_PRESENT:
            gfx_task_graph_run_present(cmdBuffer, rp);
            break;
        case GFX_RENDER_GRAPH_QUEUE_NONE:
        default:
            core_not_implemented();
            break;
        }
    }
    task_graph_log("tg end:\n");
}

void gfx_task_graph_build(){
    s_tg = (GfxTaskGraph){};
    s_tgRenderPassArena.count = 0;

    const GfxImageHandle colorHandle = s_tgPools.gfxResourceInfos[s_resources.color].resourceHandle;
    const GfxImageHandle depthStencilHandle = s_tgPools.gfxResourceInfos[s_resources.depthStencil].resourceHandle;
    const GfxImageHandle external_swapchainImage = s_tgPools.gfxResourceInfos[s_resources.external_swapchainImage[gfx_swap_chain_index()]].resourceHandle;
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_clear_depth_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_clear_depth_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (PassCtx){
            .binding = 0,
            .handle = colorHandle,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .acess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .clear = true,
        };
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (PassCtx){
            .binding = TG_INVALID,
            .handle = depthStencilHandle,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .acess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .clear = true,
        };
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_lit_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_lit_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (PassCtx){
            .binding = 0,
            .handle = colorHandle,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .acess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        };
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (PassCtx){
            .binding = TG_INVALID,
            .handle = depthStencilHandle,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .acess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        };
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_imgui_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_imgui_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (PassCtx){
            .binding = 0,
            .handle = colorHandle,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .acess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        };
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (PassCtx){
            .binding = TG_INVALID,
            .handle = depthStencilHandle,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .acess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        };
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_blit_to_swapchain");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_BLIT;
        rp->dispatch = gfx_vulkan_blit_image_to_swapchain_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->infoCtx.read[rp->infoCtx.readCount++] =   (PassCtx) {
            .binding = 0,
            .handle = colorHandle,
            .layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .acess = VK_ACCESS_TRANSFER_READ_BIT,
            .stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        };
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (PassCtx){
            .binding = 0,
            .handle = external_swapchainImage,
            .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .acess = VK_ACCESS_TRANSFER_WRITE_BIT,
            .stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        };
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_prepare_present_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_PRESENT;
        rp->dispatch = gfx_vulkan_prepare_present_run;
        rp->infoCtx.read[rp->infoCtx.readCount++] = (PassCtx){
            .binding = 0,
            .handle = external_swapchainImage,
            .layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .acess = VK_ACCESS_NONE,
            .stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        };
    }
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
static void gfx_resources_create(){
    s_tgPools = (struct GfxTaskGraphPools){};
    s_resources = (struct TaskGraphResourceInfoHandles){
        .color = gfx_task_graph_resource_create(GFX_RESOURCE_IMAGE_COLOR, &(GfxAttachmentInfo){.format = VK_FORMAT_UNDEFINED, .sizeType = GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE}, "res_color", nullptr),
        .depthStencil = gfx_task_graph_resource_create(GFX_RESOURCE_IMAGE_DEPTH_STENCIL, &(GfxAttachmentInfo){.format = VK_FORMAT_UNDEFINED, .sizeType = GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE}, "res_depth", nullptr),
        .external_swapchainImage[0] = gfx_task_graph_resource_create(GFX_RESOURCE_IMAGE_EXTERNAL_CB, nullptr, "external_swapchain_cb[0]", gfx_get_current_swapchain_image_data ),
        .external_swapchainImage[1] = gfx_task_graph_resource_create(GFX_RESOURCE_IMAGE_EXTERNAL_CB, nullptr, "external_swapchain_cb[1]", gfx_get_current_swapchain_image_data ),
    };
}

static void gfx_resources_cleanup(){
    for (uint32_t resourceIndex = 0; resourceIndex < s_tgPools.gfxResourcesCount; ++resourceIndex){
        if(s_tgPools.gfxResources[resourceIndex].type == GFX_RESOURCE_IMAGE_COLOR){
            gfx_resource_image_release(s_tgPools.gfxResources[resourceIndex].data.imageColor.handle);
        }
        if(s_tgPools.gfxResources[resourceIndex].type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            gfx_resource_image_release(s_tgPools.gfxResources[resourceIndex].data.imageDepthStencil.handle);
        }
    }
    s_tgPools = (struct GfxTaskGraphPools){};
    s_resources = (struct TaskGraphResourceInfoHandles){};
}

void gfx_task_graph_create(bool fullCreate){
    
    if(fullCreate){
        core_assert(!s_tgArenasAllocated);
        s_tgRenderPassArena.alloc = allocators()->cstd.alloc(1 * KAH_KiB);
        s_tgResourceArena.alloc = allocators()->cstd.alloc(1 * KAH_KiB);
        s_tgArenasAllocated = true;
    }

    s_tgRenderPassArena.count = 0;
    s_tgResourceArena.count = 0;

    gfx_resources_create();
}

void gfx_task_graph_cleanup(bool fullCleanup){
    gfx_resources_cleanup();
    if (fullCleanup) {
        core_assert(s_tgArenasAllocated);
        allocators()->cstd.free(s_tgRenderPassArena.alloc);
        allocators()->cstd.free(s_tgResourceArena.alloc);
        s_tgArenasAllocated = false;
    }
    s_tgRenderPassArena.count = 0;
    s_tgResourceArena.count = 0;
}
//=============================================================================
