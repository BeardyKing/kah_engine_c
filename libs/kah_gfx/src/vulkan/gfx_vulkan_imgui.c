#include <kah_core/defines.h>
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
#include <kah_gfx/gfx_logging.h>

#include <kah_core/assert.h>
#include <kah_core/memory.h>

#include <dcimgui.h>
#include <backends/dcimgui_impl_vulkan.h>
#include <backends/dcimgui_impl_win32.h>
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern GlobalGfx g_gfx;
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
CIMGUI_IMPL_API LRESULT cImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#elif CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
core_not_implemented();
#elif CHECK_FEATURE(FEATURE_PLATFORM_APPLE)
core_not_implemented();
#endif

static VkDescriptorPool s_imguiDescriptorPool = {VK_NULL_HANDLE};
static bool s_imguiFinishedRendering = {true};
#if KAH_DEBUG
static size_t s_imguiCurrentAllocationAmount = 0;
static size_t s_imguiCurrentAllocationCount = 0;
#endif //KAH_DEBUG
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static void gfx_imgui_set_theme(){
    ImGui_GetStyle()->FrameRounding = 0.0f;
    ImGui_GetStyle()->GrabRounding = 4.0f;

    ImVec4* colors = ImGui_GetStyle()->Colors;
    colors[ImGuiCol_Text] = (ImVec4){0.95f, 0.96f, 0.98f, 1.00f};
    colors[ImGuiCol_TextDisabled] = (ImVec4){0.36f, 0.42f, 0.47f, 1.00f};
    colors[ImGuiCol_WindowBg] = (ImVec4){0.11f, 0.15f, 0.17f, 1.00f};
    colors[ImGuiCol_ChildBg] = (ImVec4){0.15f, 0.18f, 0.22f, 1.00f};
    colors[ImGuiCol_PopupBg] = (ImVec4){0.08f, 0.08f, 0.08f, 0.94f};
    colors[ImGuiCol_Border] = (ImVec4){0.08f, 0.10f, 0.12f, 1.00f};
    colors[ImGuiCol_BorderShadow] = (ImVec4){0.00f, 0.00f, 0.00f, 0.00f};
    colors[ImGuiCol_FrameBg] = (ImVec4){0.20f, 0.25f, 0.29f, 1.00f};
    colors[ImGuiCol_FrameBgHovered] = (ImVec4){0.12f, 0.20f, 0.28f, 1.00f};
    colors[ImGuiCol_FrameBgActive] = (ImVec4){0.09f, 0.12f, 0.14f, 1.00f};
    colors[ImGuiCol_TitleBg] = (ImVec4){0.09f, 0.12f, 0.14f, 1.0f};
    colors[ImGuiCol_TitleBgActive] = (ImVec4){0.08f, 0.10f, 0.12f, 1.00f};
    colors[ImGuiCol_TitleBgCollapsed] = (ImVec4){0.00f, 0.00f, 0.00f, 0.51f};
    colors[ImGuiCol_MenuBarBg] = (ImVec4){0.15f, 0.18f, 0.22f, 1.00f};
    colors[ImGuiCol_ScrollbarBg] = (ImVec4){0.02f, 0.02f, 0.02f, 0.39f};
    colors[ImGuiCol_ScrollbarGrab] = (ImVec4){0.20f, 0.25f, 0.29f, 1.00f};
    colors[ImGuiCol_ScrollbarGrabHovered] = (ImVec4){0.18f, 0.22f, 0.25f, 1.00f};
    colors[ImGuiCol_ScrollbarGrabActive] = (ImVec4){0.09f, 0.21f, 0.31f, 1.00f};
    colors[ImGuiCol_CheckMark] = (ImVec4){0.28f, 0.56f, 1.00f, 1.00f};
    colors[ImGuiCol_SliderGrab] = (ImVec4){0.28f, 0.56f, 1.00f, 1.00f};
    colors[ImGuiCol_SliderGrabActive] = (ImVec4){0.37f, 0.61f, 1.00f, 1.00f};
    colors[ImGuiCol_Button] = (ImVec4){0.20f, 0.25f, 0.29f, 1.00f};
    colors[ImGuiCol_ButtonHovered] = (ImVec4){0.28f, 0.56f, 1.00f, 1.00f};
    colors[ImGuiCol_ButtonActive] = (ImVec4){0.06f, 0.53f, 0.98f, 1.00f};
    colors[ImGuiCol_Header] = (ImVec4){0.20f, 0.25f, 0.29f, 0.55f};
    colors[ImGuiCol_HeaderHovered] = (ImVec4){0.26f, 0.59f, 0.98f, 0.80f};
    colors[ImGuiCol_HeaderActive] = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
    colors[ImGuiCol_Separator] = (ImVec4){0.20f, 0.25f, 0.29f, 1.00f};
    colors[ImGuiCol_SeparatorHovered] = (ImVec4){0.10f, 0.40f, 0.75f, 0.78f};
    colors[ImGuiCol_SeparatorActive] = (ImVec4){0.10f, 0.40f, 0.75f, 1.00f};
    colors[ImGuiCol_ResizeGrip] = (ImVec4){0.26f, 0.59f, 0.98f, 0.25f};
    colors[ImGuiCol_ResizeGripHovered] = (ImVec4){0.26f, 0.59f, 0.98f, 0.67f};
    colors[ImGuiCol_ResizeGripActive] = (ImVec4){0.26f, 0.59f, 0.98f, 0.95f};
    colors[ImGuiCol_Tab] = (ImVec4){.11f, 0.15f, 0.17f, 1.00f};
    colors[ImGuiCol_TabHovered] = (ImVec4){0.26f, 0.59f, 0.98f, 0.80f};
    // colors[ImGuiCol_TabActive] = (ImVec4){0.20f, 0.25f, 0.29f, 1.00f};
    // colors[ImGuiCol_TabUnfocused] = (ImVec4){0.11f, 0.15f, 0.17f, 1.00f};
    // colors[ImGuiCol_TabUnfocusedActive] = (ImVec4){0.11f, 0.15f, 0.17f, 1.00f};
    colors[ImGuiCol_PlotLines] = (ImVec4){0.61f, 0.61f, 0.61f, 1.00f};
    colors[ImGuiCol_PlotLinesHovered] = (ImVec4){1.00f, 0.43f, 0.35f, 1.00f};
    colors[ImGuiCol_PlotHistogram] = (ImVec4){0.90f, 0.70f, 0.00f, 1.00f};
    colors[ImGuiCol_PlotHistogramHovered] = (ImVec4){1.00f, 0.60f, 0.00f, 1.00f};
    colors[ImGuiCol_TextSelectedBg] = (ImVec4){0.26f, 0.59f, 0.98f, 0.35f};
    colors[ImGuiCol_DragDropTarget] = (ImVec4){1.00f, 1.00f, 0.00f, 0.90f};
    // colors[ImGuiCol_NavHighlight] = (ImVec4){0.26f, 0.59f, 0.98f, 1.00f};
    colors[ImGuiCol_NavWindowingHighlight] = (ImVec4){1.00f, 1.00f, 1.00f, 0.70f};
    colors[ImGuiCol_NavWindowingDimBg] = (ImVec4){0.80f, 0.80f, 0.80f, 0.20f};
    colors[ImGuiCol_ModalWindowDimBg] = (ImVec4){0.80f, 0.80f, 0.80f, 0.35f};
}

static void* gfx_imgui_cstd_alloc(size_t sz, void* /*user_data*/ ){
#if KAH_DEBUG
    s_imguiCurrentAllocationAmount += sz;
    s_imguiCurrentAllocationCount++;
    gfx_log_verbose("imgui alloc size: %zu - total: %zu - count: %zu\n", sz, s_imguiCurrentAllocationAmount, s_imguiCurrentAllocationCount);
#endif //#if KAH_DEBUG
    return mem_cstd_alloc(sz)->bufferAddress;
}

static void gfx_imgui_mem_cstd_find_alloc_info(void* ptr, void* /*user_data*/ ){
    AllocInfo* info = mem_cstd_find_alloc_info(ptr);
#if KAH_DEBUG
    s_imguiCurrentAllocationAmount -= info->commitedMemory;
    s_imguiCurrentAllocationCount--;
#endif //#if KAH_DEBUG
    mem_cstd_free(info);
}
//=============================================================================

//===API=======================================================================
void gfx_imgui_begin(){
    if (s_imguiFinishedRendering){
        cImGui_ImplVulkan_NewFrame();
#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
        cImGui_ImplWin32_NewFrame();
#elif CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
        core_not_implemented();
#elif CHECK_FEATURE(FEATURE_PLATFORM_APPLE)
        core_not_implemented();
#endif
        ImGui_NewFrame();
        s_imguiFinishedRendering = false;
    }
}

void gfx_imgui_end(){
    ImGui_EndFrame();
    s_imguiFinishedRendering = true;
}

void gfx_imgui_demo_window(){
    bool show = true;
    ImGui_ShowDemoWindow(&show);
}

void gfx_imgui_draw(VkCommandBuffer cmdBuffer){
    ImGui_Render();
    cImGui_ImplVulkan_RenderDrawDataEx(ImGui_GetDrawData(), cmdBuffer, nullptr);
    s_imguiFinishedRendering = true;
}

void *gfx_imgui_get_proc_function_pointer() {
#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
    return (void *) cImGui_ImplWin32_WndProcHandler;
#elif CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
    core_not_implemented();
#elif CHECK_FEATURE(FEATURE_PLATFORM_APPLE)
    core_not_implemented();
#endif
    core_not_implemented();
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_imgui_create(void* windowHandle){
    ImGui_SetAllocatorFunctions(gfx_imgui_cstd_alloc, gfx_imgui_mem_cstd_find_alloc_info, nullptr);

    constexpr uint32_t poolSize = 11;
    const VkDescriptorPoolSize descriptorPoolSizes[poolSize] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    const VkDescriptorPoolCreateInfo descriptorPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = poolSize,
        .pPoolSizes = descriptorPoolSizes,
    };

    const VkResult poolResult = vkCreateDescriptorPool(g_gfx.device, &descriptorPoolInfo, g_gfx.allocationCallbacks, &s_imguiDescriptorPool);
    core_assert(poolResult == VK_SUCCESS);

    ImGui_CreateContext(nullptr);

#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
    cImGui_ImplWin32_Init(windowHandle);
#elif CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
    core_not_implemented();
#elif CHECK_FEATURE(FEATURE_PLATFORM_APPLE)
    core_not_implemented();
#endif

    ImGuiIO* io = ImGui_GetIO();
    io->ConfigFlags = ImGuiConfigFlags_None;
    {
        io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    }
    io->BackendFlags = ImGuiBackendFlags_None;
    {
        io->BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io->BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    }

    gfx_imgui_set_theme();

    VkFormat targetdepthFormat = gfx_vulkan_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);
    VkFormat targetColourFormat = gfx_vulkan_utils_select_surface_format().format;
    ImGui_ImplVulkan_InitInfo imguiInitInfo = {
        .Instance = g_gfx.instance,
        .PhysicalDevice = g_gfx.physicalDevice,
        .Device = g_gfx.device,
        .Queue = g_gfx.queue,
        .DescriptorPool = s_imguiDescriptorPool,
        .MinImageCount = 3,
        .ImageCount = 3,
        .MSAASamples = g_gfx.sampleCount,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &targetColourFormat,
            .depthAttachmentFormat = targetdepthFormat,
            .stencilAttachmentFormat = targetdepthFormat,
        },
    };

    cImGui_ImplVulkan_Init(&imguiInitInfo);
}

void gfx_imgui_cleanup(){
    cImGui_ImplVulkan_Shutdown();
#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
    cImGui_ImplWin32_Shutdown();
#elif CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
    core_not_implemented();
#elif CHECK_FEATURE(FEATURE_PLATFORM_APPLE)
    core_not_implemented();
#endif
    ImGui_DestroyContext(nullptr);
    vkDestroyDescriptorPool(g_gfx.device, s_imguiDescriptorPool, g_gfx.allocationCallbacks);
}
//=============================================================================
#endif // #if CHECK_FEATURE(FEATURE_GFX_IMGUI)