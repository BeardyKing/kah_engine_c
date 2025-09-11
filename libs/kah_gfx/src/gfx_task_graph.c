//===INCLUDES==================================================================
#include <kah_gfx/gfx_task_graph.h>
#include <kah_gfx/gfx_logging.h>
#include <kah_gfx/gfx_pool.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>

#include <kah_core/allocators.h>
#include <kah_core/assert.h>
#include <kah_core/utils.h>

#include <stdio.h>
//=============================================================================
constexpr size_t TG_RENDER_PASS_MAX = 128;
constexpr size_t TG_ATTACHMENTS_MAX = 128;
constexpr size_t TG_RENDER_PASS_NAME_MAX = 64;
constexpr size_t TG_RENDER_PASS_ATTACHMENT_MAX = 8;
constexpr char TG_ARENA_STRING_FALLBACK[TG_RENDER_PASS_NAME_MAX] = "err: no tg string memory remaining";

struct GfxTaskGraph typedef GfxTaskGraph;
struct GfxRenderPass typedef GfxRenderPass;

typedef void (*GfxRenderPassRun_cb)(VkCommandBuffer cmd, VkRenderingInfoKHR renderingInfo);

enum GfxRenderGraphQueueType {
    GFX_RENDER_GRAPH_QUEUE_GRAPHICS = 1 << 0,
    GFX_RENDER_GRAPH_QUEUE_COMPUTE = 1 << 1,
    GFX_RENDER_GRAPH_QUEUE_ASYNC_COMPUTE = 1 << 2,
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

    uint32_t depthStencilAttachmentHandle;
    uint32_t colorAttachmentHandles[TG_RENDER_PASS_ATTACHMENT_MAX];
    uint32_t colorAttachmentCount;

    GfxAttachmentSizeType sizeType;
    vec2u attachmentSize;
}typedef GfxRenderPass;

struct GfxTaskGraph {
    GfxRenderPass renderPasses[TG_RENDER_PASS_MAX];
    uint32_t renderPassCount;

    VkRenderingAttachmentInfoKHR attachments[TG_ATTACHMENTS_MAX];
    uint32_t attachmentCount;
} typedef GfxTaskGraph;
//=============================================================================

//===LOCAL_VARIABLES===========================================================
static GfxTaskGraph s_tg = {};

static struct TaskGraphStringArena{
    AllocInfo* alloc;
    size_t count;
} s_tgStringArena = {};
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
//=============================================================================

//===API=======================================================================
void gfx_task_graph_run(VkCommandBuffer cmdBuffer){
    for (uint32_t renderPassIndx = 0; renderPassIndx < s_tg.renderPassCount; ++renderPassIndx){
        GfxRenderPass* rp = &s_tg.renderPasses[renderPassIndx];
        gfx_log_verbose("render pass: %s\n", rp->name);

        core_assert(rp->sizeType != ATTACHMENT_NONE || (rp->attachmentSize.x == 0 && rp->attachmentSize.y == 0));
        core_assert(rp->sizeType != ATTACHMENT_ABSOLUTE || (rp->attachmentSize.x != 0 && rp->attachmentSize.y != 0));
        const vec2u size = rp->sizeType == ATTACHMENT_SWAPCHAIN ? gfx_vulkan_swapchain_size() : rp->attachmentSize;

        core_assert(rp->colorAttachmentCount < TG_RENDER_PASS_ATTACHMENT_MAX);
        const VkRenderingAttachmentInfoKHR depthStencilAttachment = s_tg.attachments[rp->depthStencilAttachmentHandle];
        VkRenderingAttachmentInfoKHR colorAttachments[TG_RENDER_PASS_ATTACHMENT_MAX] = {};
        for (uint32_t attachmentIndex = 0; attachmentIndex< rp->colorAttachmentCount; ++attachmentIndex){
            colorAttachments[attachmentIndex] = s_tg.attachments[rp->colorAttachmentHandles[attachmentIndex]];
        }

        const VkRenderingInfoKHR renderingInfo = (VkRenderingInfoKHR){
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {.offset = {0, 0}, .extent = {size.x, size.y}},
            .layerCount = 1,
            .colorAttachmentCount = rp->colorAttachmentCount,
            .pColorAttachments = colorAttachments,
            .pDepthAttachment = &depthStencilAttachment,
            .pStencilAttachment = &depthStencilAttachment,
        };

        rp->dispatch(cmdBuffer, renderingInfo);
    }

    s_tg = (GfxTaskGraph){};
    s_tgStringArena.count = 0;
}

uint32_t gfx_task_graph_push_attachment_info(VkRenderingAttachmentInfoKHR info){
    s_tg.attachments[s_tg.attachmentCount] = info;
    return s_tg.attachmentCount++;
}

void gfx_task_graph_build(){
    const uint32_t swapchainHandle = gfx_task_graph_push_attachment_info( get_swapchain_color_attachment_info());
    const uint32_t targetColourHandle = gfx_task_graph_push_attachment_info( get_render_target_color_attachment_info());
    const uint32_t depthStencilHandle = gfx_task_graph_push_attachment_info( get_render_target_depth_stencil_attachment_info());

    {
        GfxRenderPass* rpClear = render_pass_create("gfx_vulkan_clear_depth_run");
        rpClear->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rpClear->dispatch = gfx_vulkan_clear_depth_run;
        rpClear->sizeType = ATTACHMENT_SWAPCHAIN;
        rpClear->colorAttachmentHandles[rpClear->colorAttachmentCount++] = targetColourHandle;
        rpClear->depthStencilAttachmentHandle = depthStencilHandle;
    }
    {
        GfxRenderPass* rpImGui = render_pass_create("gfx_vulkan_imgui_run");
        rpImGui->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rpImGui->dispatch = gfx_vulkan_imgui_run;
        rpImGui->sizeType = ATTACHMENT_SWAPCHAIN;
        rpImGui->colorAttachmentHandles[rpImGui->colorAttachmentCount++] = targetColourHandle;
        rpImGui->depthStencilAttachmentHandle = depthStencilHandle;
    }
    {
        GfxRenderPass* blitImageToSwapchain = render_pass_create("gfx_blit_to_swapchain");
        blitImageToSwapchain->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        blitImageToSwapchain->dispatch = gfx_vulkan_blit_image_to_swapchain_run;
        blitImageToSwapchain->sizeType = ATTACHMENT_SWAPCHAIN;
        blitImageToSwapchain->colorAttachmentHandles[blitImageToSwapchain->colorAttachmentCount++] = swapchainHandle;
        blitImageToSwapchain->colorAttachmentHandles[blitImageToSwapchain->colorAttachmentCount++] = targetColourHandle;
    }
    {
        GfxRenderPass* rpPresent = render_pass_create("gfx_vulkan_prepare_present_run");
        rpPresent->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rpPresent->dispatch = gfx_vulkan_prepare_present_run;
    }
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_task_graph_create(){
    s_tgStringArena.alloc = allocators()->cstd.alloc(1 * KAH_KiB);
    s_tgStringArena.count = 0;
}

void gfx_task_graph_cleanup(){
    allocators()->cstd.free(s_tgStringArena.alloc);
    s_tgStringArena.count = 0;
}
//=============================================================================
