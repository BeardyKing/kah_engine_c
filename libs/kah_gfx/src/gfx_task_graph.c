//===INCLUDES==================================================================
#include <kah_gfx/gfx_task_graph.h>
#include <kah_gfx/gfx_logging.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>

#include <kah_core/allocators.h>
#include <kah_core/assert.h>
#include <kah_core/utils.h>

#include <stdio.h>
//=============================================================================
constexpr size_t TG_RENDER_PASS_MAX = 128;
constexpr size_t TG_RENDER_PASS_NAME_MAX = 64;
constexpr char TG_ARENA_STRING_FALLBACK[TG_RENDER_PASS_NAME_MAX] = "err: no tg string memory remaining";

struct GfxTaskGraph typedef GfxTaskGraph;
struct GfxRenderPass typedef GfxRenderPass;

typedef void (*GfxRenderPassRun_cb)(VkCommandBuffer cmd);

enum GfxSizeType {
    SIZE_TYPE_ABSOLUTE,
    SIZE_TYPE_SWAPCHAIN_RELATIVE,
    SIZE_TYPE_INPUT_RELATIVE
}typedef GfxSizeType;

enum GfxRenderGraphQueueType {
    GFX_RENDER_GRAPH_QUEUE_GRAPHICS = 1 << 0,
    GFX_RENDER_GRAPH_QUEUE_COMPUTE = 1 << 1,
    GFX_RENDER_GRAPH_QUEUE_ASYNC_COMPUTE = 1 << 2,
}typedef GfxRenderGraphQueueType;

struct GfxRenderPass {
    GfxRenderGraphQueueType queue;
    const char* name;
    GfxRenderPassRun_cb dispatch;
}typedef GfxRenderPass;

struct GfxTaskGraph {
    GfxRenderPass renderPasses[TG_RENDER_PASS_MAX];
    uint32_t renderPassCount;
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
    for (uint32_t i = 0; i < s_tg.renderPassCount; ++i){
        gfx_log_verbose("render pass: %s\n", s_tg.renderPasses[i].name);
        s_tg.renderPasses[i].dispatch(cmdBuffer);
    }
}

void gfx_task_graph_build(){
    {
        GfxRenderPass* rpClear = render_pass_create("gfx_vulkan_clear_depth_run");
        rpClear->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rpClear->dispatch = gfx_vulkan_clear_depth_run;
    }
    {
        GfxRenderPass* rpImGui = render_pass_create("gfx_vulkan_imgui_run");
        rpImGui->queue = GFX_RENDER_GRAPH_QUEUE_GRAPHICS;
        rpImGui->dispatch = gfx_vulkan_imgui_run;
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
