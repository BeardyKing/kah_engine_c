//===INCLUDES==================================================================
#include <kah_gfx/gfx_task_graph.h>
#include <kah_gfx/gfx_logging.h>
#include <kah_gfx/gfx_pool.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_resource.h>

#include <kah_core/allocators.h>
#include <kah_core/assert.h>
#include <kah_core/utils.h>

#include <stdio.h>
//=============================================================================

//===LOCAL_STRUCTS=============================================================
constexpr size_t TG_RENDER_PASS_MAX = 128;
constexpr size_t TG_ATTACHMENTS_MAX = 128;
constexpr size_t TG_RENDER_PASS_NAME_MAX = 64;
constexpr size_t TG_RENDER_PASS_ATTACHMENT_MAX = 8;
constexpr char TG_ARENA_STRING_FALLBACK[TG_RENDER_PASS_NAME_MAX] = "err: no tg string memory remaining";

struct GfxTaskGraph typedef GfxTaskGraph;
struct GfxRenderPass typedef GfxRenderPass;

typedef void (*GfxRenderPassRun_cb)(VkCommandBuffer cmd, VkRenderingInfoKHR renderingInfo, GfxRenderContext ctx);
typedef uint32_t GfxResourceInfoHandle;

enum GfxRenderGraphQueueType {
    GFX_RENDER_GRAPH_QUEUE_NONE = 0,
    GFX_RENDER_GRAPH_QUEUE_GRAPHICS = 1 << 0,
    GFX_RENDER_GRAPH_QUEUE_BLIT = 1 << 1,
    // GFX_RENDER_GRAPH_QUEUE_COMPUTE = 1 << 2,
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
} typedef GfxResourceInfo;

struct GfxRenderInfoContext{
    struct ReadCtx{
        uint32_t binding;
        GfxResourceInfoHandle handle;
    }read[GFX_RENDER_CONTEXT_READ_WRITE_MAX];
    uint32_t readCount;

    struct WriteCtx{
        uint32_t binding;
        GfxResourceInfoHandle handle;
    }write[GFX_RENDER_CONTEXT_READ_WRITE_MAX];
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
    GfxResourceInfoHandle external_swapchainImage;
} s_resources;

static struct TaskGraphStringArena{
    AllocInfo* alloc;
    size_t count;
} s_tgStringArena = {};

constexpr uint32_t TG_RESOURCE_POOL_MAX = 32;
static struct GfxTaskGraphPools{
    GfxResource gfxResources[TG_RESOURCE_POOL_MAX];
    uint32_t gfxResourcesCount;

    GfxResourceInfo gfxResourceInfos[TG_RESOURCE_POOL_MAX];
    uint32_t gfxResourceInfosCount;
}s_tgPools = {};
//=============================================================================

//===LOCAL_FUNCTIONS===========================================================
static const char* task_graph_arena_string(const char* inStr){
    core_assert(s_tgStringArena.alloc);
    const size_t inStrLen = strlen(inStr) + 1;
    if((s_tgStringArena.count + inStrLen) < s_tgStringArena.alloc->commitedMemory){
        core_assert(inStrLen < TG_RENDER_PASS_NAME_MAX);
        char* outStr = &((char*)s_tgStringArena.alloc->bufferAddress)[s_tgStringArena.count];
        s_tgStringArena.count += inStrLen;
        sprintf(outStr, "%s\0", inStr);
        return outStr;
    }
    return TG_ARENA_STRING_FALLBACK;
}

static GfxRenderPass* render_pass_create(const char* renderPassName){
    core_assert(s_tg.renderPassCount + 1 < TG_RENDER_PASS_MAX);
    GfxRenderPass* outRP = &s_tg.renderPasses[s_tg.renderPassCount++];
    outRP->name = task_graph_arena_string(renderPassName);
    return outRP;
}

GfxResourceInfoHandle gfx_task_graph_resource_create(GfxResourceType type, GfxAttachmentInfo* attachmentInfo, const char* resourceName, GfxImageExternal_cb cb){
    core_assert(s_tgPools.gfxResourcesCount + 1 < TG_RESOURCE_POOL_MAX);
    core_assert(s_tgPools.gfxResourceInfosCount + 1 < TG_RESOURCE_POOL_MAX);
    const uint32_t outResourcePoolIndex = s_tgPools.gfxResourcesCount;
    const uint32_t outResourceInfoIndex = s_tgPools.gfxResourceInfosCount;
    {
        GfxResource* resource = &s_tgPools.gfxResources[s_tgPools.gfxResourcesCount++];

        if(type == GFX_RESOURCE_IMAGE_COLOR){
            const uint32_t resourceDataHandle = gfx_resource_create_type(type, attachmentInfo);
            *resource = (GfxResource){.type = type, .data.imageColor = {.binding = GFX_RESOURCE_BINDING_NONE, .handle = resourceDataHandle}};
        }
        if(type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            const uint32_t resourceDataHandle = gfx_resource_create_type(type, attachmentInfo);
            *resource = (GfxResource){.type = type, .data.imageDepthStencil = {.handle = resourceDataHandle}};
        }
        if(type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
            core_assert(cb != nullptr);
            *resource = (GfxResource){.type = type, .data.external = {.binding = GFX_RESOURCE_BINDING_NONE, .imageCB = cb}};
        }
    }
    {
        s_tgPools.gfxResourceInfos[s_tgPools.gfxResourceInfosCount++] = (GfxResourceInfo){
            .name = task_graph_arena_string(resourceName),
            .resourceHandle = outResourcePoolIndex,
            .attachmentInfo = attachmentInfo != nullptr ? *attachmentInfo : (GfxAttachmentInfo){},
            .lastLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .lastAccess = 0 // not a valid enum entry.
        };
    }
    return outResourceInfoIndex;
}


static void build_render_context_from_info_context(GfxRenderContext* ctx, const GfxRenderInfoContext* info){
    core_assert(ctx != nullptr);
    core_assert(info != nullptr);

    ctx->readCount = info->readCount;
    ctx->writeCount = info->writeCount;

    for (uint32_t readIndex = 0; readIndex < info->readCount; ++readIndex){
        const uint32_t bindingIndex = info->read[readIndex].binding;
        ctx->read[readIndex] = s_tgPools.gfxResources[info->read[readIndex].handle];

        if(ctx->read[readIndex].type == GFX_RESOURCE_IMAGE_COLOR ){
            ctx->read[readIndex].data.imageColor.binding = bindingIndex;
        }
        if(ctx->read[readIndex].type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            core_assert(bindingIndex == GFX_RESOURCE_BINDING_NONE);
        }
        if(ctx->read[readIndex].type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
            ctx->read[readIndex].data.external.binding = bindingIndex;
        }
    }

    for (uint32_t writeIndex = 0; writeIndex < info->writeCount; ++writeIndex){
        const uint32_t bindingIndex = info->write[writeIndex].binding;
        ctx->write[writeIndex] = s_tgPools.gfxResources[info->write[writeIndex].handle];

        if(ctx->write[writeIndex].type == GFX_RESOURCE_IMAGE_COLOR ){
            ctx->write[writeIndex].data.imageColor.binding = bindingIndex;
        }
        if(ctx->write[writeIndex].type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            core_assert(bindingIndex == GFX_RESOURCE_BINDING_NONE);
        }
        if(ctx->write[writeIndex].type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
            ctx->write[writeIndex].data.external.binding = bindingIndex;
        }
    }
}

static void gfx_task_graph_run_blit(VkCommandBuffer cmdBuffer, const GfxRenderPass* rp){
    GfxRenderContext renderCtx = {};
    build_render_context_from_info_context(&renderCtx, &rp->infoCtx);
    rp->dispatch(cmdBuffer, (VkRenderingInfoKHR){}, renderCtx);
}

static void gfx_task_graph_run_graphics(VkCommandBuffer cmdBuffer, const GfxRenderPass* rp){
    gfx_log_verbose("render pass graphics: %s\n", rp->name);

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
        if(type == GFX_RESOURCE_IMAGE_COLOR){
            const uint32_t binding = renderCtx.write[writeIndex].data.imageColor.binding;
            core_assert(colorAttachmentsInUse[binding] == false);
            colorAttachments[binding] = gfx_rendering_attachment_info_color(gfx_pool_get_gfx_image(renderCtx.write[writeIndex].data.imageColor.handle)->view);
            colorAttachmentsInUse[binding] = true;
            colorAttachmentCount = max(colorAttachmentCount, binding + 1);
        }
        if(type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            core_assert(depthStencilAttachmentInUse == false);
            depthStencilAttachment = gfx_rendering_attachment_info_depth_spencil(gfx_pool_get_gfx_image(renderCtx.write[writeIndex].data.imageDepthStencil.handle)->view);
            depthStencilAttachmentInUse = true;
        }
        if(type == GFX_RESOURCE_IMAGE_EXTERNAL_CB){
            // Note: we probably want (some) external resources to be included in the color attachment count,
            // but we don't sore that info currently.
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

    //TODO: batch barriers here:
    //TODO: gfx_command_begin_rendering here:
    rp->dispatch(cmdBuffer, renderingInfo, renderCtx);
    //TODO: gfx_command_end_rendering here:
}
//=============================================================================

//===API=======================================================================
void gfx_task_graph_run(VkCommandBuffer cmdBuffer){
    gfx_log_verbose("tg begin:\n");
    for (uint32_t renderPassIndex = 0; renderPassIndex < s_tg.renderPassCount; ++renderPassIndex){
        const GfxRenderPass* rp = &s_tg.renderPasses[renderPassIndex];
        gfx_log_verbose("rp begin: %s\n",rp->name);
        core_assert(rp->infoCtx.readCount <= GFX_RENDER_CONTEXT_READ_WRITE_MAX);
        core_assert(rp->infoCtx.writeCount <= GFX_RENDER_CONTEXT_READ_WRITE_MAX);

        switch (rp->queue){
        case GFX_RENDER_GRAPH_QUEUE_GRAPHICS:
            gfx_task_graph_run_graphics(cmdBuffer, rp);
            break;
        case GFX_RENDER_GRAPH_QUEUE_BLIT:
            gfx_task_graph_run_blit(cmdBuffer, rp);
            break;
        case GFX_RENDER_GRAPH_QUEUE_NONE:
        default:
            core_not_implemented();
            break;
        }
    }
    gfx_log_verbose("tg end:\n");

    s_tg = (GfxTaskGraph){};
    s_tgStringArena.count = 0;
}

void gfx_task_graph_build(){
    const GfxImageHandle colorHandle = s_tgPools.gfxResourceInfos[s_resources.color].resourceHandle;
    const GfxImageHandle depthStencilHandle = s_tgPools.gfxResourceInfos[s_resources.depthStencil].resourceHandle;
    const GfxImageHandle external_swapchainImage = s_tgPools.gfxResourceInfos[s_resources.external_swapchainImage].resourceHandle;
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_clear_depth_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_clear_depth_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (struct WriteCtx){.binding = 0,                         .handle = colorHandle };
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (struct WriteCtx){.binding = GFX_RESOURCE_BINDING_NONE, .handle = depthStencilHandle };
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_imgui_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_imgui_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (struct WriteCtx){.binding = 0,                         .handle = colorHandle };
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (struct WriteCtx){.binding = GFX_RESOURCE_BINDING_NONE, .handle = depthStencilHandle };
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_blit_to_swapchain");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_BLIT;
        rp->dispatch = gfx_vulkan_blit_image_to_swapchain_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->infoCtx.read[rp->infoCtx.readCount++] =   (struct ReadCtx) {.binding = 0, .handle = colorHandle };
        rp->infoCtx.write[rp->infoCtx.writeCount++] = (struct WriteCtx){.binding = 0, .handle = external_swapchainImage};
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_prepare_present_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_prepare_present_run;
        rp->infoCtx.read[rp->infoCtx.readCount++] = (struct ReadCtx){.binding = 0, .handle = external_swapchainImage};
    }
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
static void gfx_resources_create(){
    s_tgPools = (struct GfxTaskGraphPools){};
    s_resources = (struct TaskGraphResourceInfoHandles){
        .color = gfx_task_graph_resource_create(GFX_RESOURCE_IMAGE_COLOR, &(GfxAttachmentInfo){.format = VK_FORMAT_UNDEFINED, .sizeType = GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE}, "res_color", nullptr),
        .depthStencil = gfx_task_graph_resource_create(GFX_RESOURCE_IMAGE_DEPTH_STENCIL, &(GfxAttachmentInfo){.format = VK_FORMAT_UNDEFINED, .sizeType = GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE}, "res_depth", nullptr),
        .external_swapchainImage = gfx_task_graph_resource_create(GFX_RESOURCE_IMAGE_EXTERNAL_CB, nullptr, "external_swapchain_cb", gfx_get_current_swapchain_image_data ),
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

void gfx_task_graph_create(){
    s_tgStringArena.alloc = allocators()->cstd.alloc(1 * KAH_KiB);
    s_tgStringArena.count = 0;
    gfx_resources_create();
}

void gfx_task_graph_cleanup(){
    gfx_resources_cleanup();
    allocators()->cstd.free(s_tgStringArena.alloc);
    s_tgStringArena.count = 0;
}
//=============================================================================
