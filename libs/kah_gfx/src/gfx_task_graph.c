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

enum GfxRenderGraphQueueType {
    GFX_RENDER_GRAPH_QUEUE_NONE = 1 << 0,
    GFX_RENDER_GRAPH_QUEUE_GRAPHICS = 1 << 1,
    GFX_RENDER_GRAPH_QUEUE_BLIT = 1 << 2,
    // GFX_RENDER_GRAPH_QUEUE_COMPUTE = 1 << 3,
}typedef GfxRenderGraphQueueType;

enum GfxAttachmentSizeType{
    ATTACHMENT_NONE,
    ATTACHMENT_ABSOLUTE,
    ATTACHMENT_SWAPCHAIN
}typedef GfxAttachmentSizeType;

struct GfxRenderPass {
    GfxRenderGraphQueueType queue;
    const char* name;
    GfxRenderPassRun_cb dispatch;

    GfxRenderContext ctx;

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

static struct{
    GfxImageHandle colorHandle;
    GfxImageHandle depthStencilHandle;
} s_resources;

static struct TaskGraphStringArena{
    AllocInfo* alloc;
    size_t count;
} s_tgStringArena = {};
//=============================================================================

//===LOCAL_FUNCTIONS===========================================================
static void gfx_resources_create(){
    s_resources.colorHandle = gfx_resource_image_colour_create(&(GfxAttachmentInfo){.format = VK_FORMAT_UNDEFINED, .sizeType = GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE});
    s_resources.depthStencilHandle = gfx_resource_image_depth_create(&(GfxAttachmentInfo){.format = VK_FORMAT_UNDEFINED, .sizeType = GFX_SIZE_TYPE_SWAPCHAIN_RELATIVE});
}

static void gfx_resources_cleanup(){
    gfx_resource_image_release(s_resources.colorHandle);
    gfx_resource_image_release(s_resources.depthStencilHandle);
}

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

    for (uint32_t writeIndex = 0; writeIndex < rp->ctx.writeCount; ++writeIndex){
        GfxResourceType type = rp->ctx.write[writeIndex].type;
        if(type == GFX_RESOURCE_IMAGE_COLOR){
            const uint32_t binding = rp->ctx.write[writeIndex].data.imageColor.binding;
            core_assert(colorAttachmentsInUse[binding] == false);
            colorAttachments[binding] = gfx_rendering_attachment_info_color(gfx_pool_get_gfx_image(rp->ctx.write[writeIndex].data.imageColor.handle)->view);
            colorAttachmentsInUse[binding] = true;
            colorAttachmentCount = max(colorAttachmentCount, binding + 1);
        }
        if(type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL){
            core_assert(depthStencilAttachmentInUse == false);
            depthStencilAttachment = gfx_rendering_attachment_info_depth_spencil(gfx_pool_get_gfx_image(rp->ctx.write[writeIndex].data.imageDepthStencil.handle)->view);
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
    rp->dispatch(cmdBuffer, renderingInfo, rp->ctx);
    //TODO: gfx_command_end_rendering here:
}
//=============================================================================

//===API=======================================================================
void gfx_task_graph_run(VkCommandBuffer cmdBuffer){
    for (uint32_t renderPassIndex = 0; renderPassIndex < s_tg.renderPassCount; ++renderPassIndex){
        const GfxRenderPass* rp = &s_tg.renderPasses[renderPassIndex];
        core_assert(rp->ctx.readCount <= GFX_RENDER_CONTEXT_READ_WRITE_MAX);
        core_assert(rp->ctx.writeCount <= GFX_RENDER_CONTEXT_READ_WRITE_MAX);

        switch (rp->queue){
        case GFX_RENDER_GRAPH_QUEUE_GRAPHICS:
            gfx_task_graph_run_graphics(cmdBuffer, rp);
            break;
        case GFX_RENDER_GRAPH_QUEUE_BLIT:
            rp->dispatch(cmdBuffer, (VkRenderingInfoKHR){}, rp->ctx);
            break;
        case GFX_RENDER_GRAPH_QUEUE_NONE:
        default:
            core_not_implemented();
            break;
        }
    }

    s_tg = (GfxTaskGraph){};
    s_tgStringArena.count = 0;
}

void gfx_task_graph_build(){
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_clear_depth_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_clear_depth_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;

        rp->ctx.write[rp->ctx.writeCount++] = (GfxResource){.type = GFX_RESOURCE_IMAGE_COLOR, .data.imageColor = {.binding = 0, .handle = s_resources.colorHandle}};
        rp->ctx.write[rp->ctx.writeCount++] = (GfxResource){.type = GFX_RESOURCE_IMAGE_DEPTH_STENCIL, .data.imageDepthStencil = {.handle = s_resources.depthStencilHandle}};
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_imgui_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_imgui_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->ctx.write[rp->ctx.writeCount++] = (GfxResource){.type = GFX_RESOURCE_IMAGE_COLOR, .data.imageColor = {.binding = 0, .handle = s_resources.colorHandle}};
        rp->ctx.write[rp->ctx.writeCount++] = (GfxResource){.type = GFX_RESOURCE_IMAGE_DEPTH_STENCIL, .data.imageDepthStencil = {.handle = s_resources.depthStencilHandle}};
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_blit_to_swapchain");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_BLIT;
        rp->dispatch = gfx_vulkan_blit_image_to_swapchain_run;
        rp->sizeType = ATTACHMENT_SWAPCHAIN;
        rp->ctx.read[rp->ctx.readCount++] = (GfxResource){.type = GFX_RESOURCE_IMAGE_COLOR, .data.imageColor = {.binding = 0, .handle = s_resources.colorHandle}};
        rp->ctx.write[rp->ctx.writeCount++] = (GfxResource){.type = GFX_RESOURCE_IMAGE_EXTERNAL_CB, .data.external = {.binding = 0, .imageCB = gfx_get_current_swapchain_image_data }};
    }
    {
        GfxRenderPass* rp = render_pass_create("gfx_vulkan_prepare_present_run");
        rp->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rp->dispatch = gfx_vulkan_prepare_present_run;
        rp->ctx.read[rp->ctx.readCount++] = (GfxResource){.type = GFX_RESOURCE_IMAGE_EXTERNAL_CB, .data.external = {.binding = 0, .imageCB = gfx_get_current_swapchain_image_data }};
    }
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_task_graph_create(){
    gfx_resources_create();
    s_tgStringArena.alloc = allocators()->cstd.alloc(1 * KAH_KiB);
    s_tgStringArena.count = 0;
}

void gfx_task_graph_cleanup(){
    gfx_resources_cleanup();
    allocators()->cstd.free(s_tgStringArena.alloc);
    s_tgStringArena.count = 0;
}
//=============================================================================
