
#ifndef WINDGET_MANAGER_C_H
#define WINDGET_MANAGER_C_H

//===INCLUDES==================================================================
#include <client/widgets/widget_manager.h>

#include <kah_core/assert.h>
#include <kah_core/allocators.h>
#include <kah_core/c_string.h>
#include <kah_core/core_cvars.h>
#include <kah_core/input.h>
#include <kah_core/filesystem.h>

#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/gfx_pool.h>
#include <kah_gfx/vulkan/gfx_vulkan_imgui.h>
#include <kah_gfx/vulkan/gfx_vulkan_bindless.h>
#include <kah_gfx/vulkan/gfx_vulkan_texture.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>

#include <dcimgui.h>
#include <dcimgui_impl_vulkan.h>

#include <stdio.h>
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
enum WidgetType {
    KAH_WIDGET_ACTIVE_WIDGETS_MENU = 0,
    KAH_WIDGET_TOOLBAR_NAV_MENU,
    KAH_WIDGET_CVARS,
    KAH_WIDGET_IMAGE_DIFF,
    KAH_WIDGET_IMGUI_DEMO_MENU,

    KAH_WIDGET_COUNT
}typedef WidgetType;

struct WidgetInfo {
    bool isActive;
    char name[64];
    char toolbarTabName[64];
    char shortcut[64];
} typedef WidgetInfo;

static WidgetInfo s_widgetTable[KAH_WIDGET_COUNT] = {};

struct DiffSelectionCtx {
    char dirPath[KAH_FILESYSTEM_MAX_PATH];
    int32_t dirFileCount;
    AllocInfo* fileNames;
    int32_t selectedFileIndex;
    GfxTextureHandle loadedTextureHandle;
    ImTextureRef imguiImageId;
    bool hasRun;
    struct {
        bool active;
        int counter;
        GfxTextureHandle loadedTextureHandle;
        ImTextureRef imguiImageId;
    }deferRelease;
} typedef DiffSelectionCtx;

static DiffSelectionCtx s_diffImageCtxA = {};
static DiffSelectionCtx s_diffImageCtxB = {};
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static void widget_toolbar_update() {
    WidgetInfo* navigationMenu = &s_widgetTable[KAH_WIDGET_TOOLBAR_NAV_MENU];
    if (navigationMenu->isActive && ImGui_BeginMainMenuBar()) {
        for (uint32_t i = 0; i < KAH_WIDGET_COUNT; ++i) {
            WidgetInfo* info = &s_widgetTable[i];
            if (!c_str_empty(info->toolbarTabName)) {
                if (ImGui_BeginMenuEx(info->toolbarTabName, true)) {
                    if(ImGui_MenuItemEx(info->name, info->shortcut, info->isActive, true)){
                        info->isActive = !info->isActive;
                    }
                    ImGui_EndMenu();
                }
            }
        }
        ImGui_EndMainMenuBar();
    }
}

static void widget_state_update() {
    WidgetInfo* menuWidget = &s_widgetTable[KAH_WIDGET_ACTIVE_WIDGETS_MENU];
    if (menuWidget->isActive) {
        ImGui_SetNextWindowSize((ImVec2){.x = 275,.y = 200}, ImGuiCond_FirstUseEver);
        ImGui_Begin(menuWidget->name, &menuWidget->isActive, ImGuiWindowFlags_None);
        {
            for (uint32_t i = 0; i < KAH_WIDGET_COUNT; ++i) {
                WidgetInfo* currentWidget = &s_widgetTable[i];
                if(!c_str_empty(currentWidget->name)){
                    ImGui_Checkbox(currentWidget->name, &currentWidget->isActive);
                }
            }
        }
        ImGui_End();
    }
}

static void cvar_debug_vec2i(const char* varName, vec2i_cvar_t* cvar) {
    char textBuf[128] = {};
    sprintf_s(textBuf, 128, "%s - vec2i{%i,%i}###%s_cvar_header", varName, cvar->current.x, cvar->current.y, varName);
    if(ImGui_CollapsingHeader(textBuf, ImGuiTreeNodeFlags_None)){
        ImGui_Text("min (%i , %i) - max (%i , %i)", cvar->min.x, cvar->min.y, cvar->max.x, cvar->max.y);
        
        float spacing = ImGui_GetStyle()->ItemSpacing.x;
        float avail   = ImGui_GetContentRegionAvail().x;
        float half = (avail - spacing) * 0.5f;

        ImGui_PushItemWidth(half);
        sprintf_s(textBuf, 128, "##cvar_current_x_%s", varName);
        ImGui_DragIntEx(textBuf, &cvar->current.x, 1, cvar->min.x, cvar->max.x, ".x = (%i)", ImGuiSliderFlags_AlwaysClamp);
        ImGui_PopItemWidth();
        ImGui_SameLineEx(0.0f, spacing);
        ImGui_PushItemWidth(half);
        sprintf_s(textBuf, 128, "##cvar_current_y_%s", varName);
        ImGui_DragIntEx(textBuf, &cvar->current.y, 1, cvar->min.y, cvar->max.y, ".y = (%i)", ImGuiSliderFlags_AlwaysClamp);
        ImGui_PopItemWidth();
    }
}

static void widget_cvar_update(){
    WidgetInfo* cvarWidget = &s_widgetTable[KAH_WIDGET_CVARS];
    if(input_is_key_pressed(KEYCODE_GRAVEACCENT)){
        cvarWidget->isActive = !cvarWidget->isActive;
    }
    if (cvarWidget->isActive){
        if (ImGui_Begin(cvarWidget->name, &cvarWidget->isActive, ImGuiWindowFlags_None)){
            cvar_debug_vec2i("windowSize", g_coreCvars.windowSize);
            cvar_debug_vec2i("windowPosition", g_coreCvars.windowPosition);
            ImGui_End();
        }
    }
}

static void widget_image_diff_selection(DiffSelectionCtx* diffCtx){
    char colString[128];
    sprintf(colString, "##%p", diffCtx);
    if (ImGui_InputText(colString, &diffCtx->dirPath[0], KAH_FILESYSTEM_MAX_PATH, ImGuiInputFlags_None) || !diffCtx->hasRun) {
        diffCtx->hasRun = true;
        diffCtx->dirFileCount = fs_dir_file_count(diffCtx->dirPath);
        if (diffCtx->dirFileCount >= 0) {
            if (diffCtx->fileNames != NULL) {
                allocators()->cstd.free(diffCtx->fileNames);
                diffCtx->fileNames = NULL;
            }
            diffCtx->fileNames = allocators()->cstd.alloc(diffCtx->dirFileCount * (sizeof(char) * KAH_FILESYSTEM_MAX_PATH + 1));
            fs_dir_get_filenames((const char*)& diffCtx->dirPath[0], (char*)diffCtx->fileNames->bufferAddress, diffCtx->dirFileCount);
        }
    }

    if (diffCtx->dirFileCount == KAH_FILESYSTEM_INVALID) {
        ImGui_Text("invalid path");
    }
    else {
        if (ImGui_BeginTable("fileTable", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            char colString[128];
            sprintf(colString, "Files [%i] - Selected[%i]##\n", diffCtx->dirFileCount, diffCtx->selectedFileIndex);
            ImGui_TableSetupColumn(colString, ImGuiTableColumnFlags_None);
            ImGui_TableHeadersRow();

            for (int32_t i = 0; i < diffCtx->dirFileCount; i++) {
                ImGui_TableNextRow();
                ImGui_TableSetColumnIndex(0);

                const char* fileName = &((const char*)diffCtx->fileNames->bufferAddress)[sizeof(char) * i * KAH_FILESYSTEM_MAX_PATH];
                bool foundValidExt = c_str_search_reverse((char*)fileName, ".dds") != NULL;
                
                ImVec4 color = foundValidExt ? (ImVec4){ 0.2f, 1.0f, 0.2f, 1.0f } : (ImVec4){ 1.0f, 0.2f, 0.2f, 1.0f }; 
                ImGui_PushStyleColorImVec4(ImGuiCol_Text, color);

                bool isSelected = (diffCtx->selectedFileIndex == (int)i);
                if (ImGui_SelectableEx(fileName, isSelected, ImGuiSelectableFlags_SpanAllColumns, (ImVec2) { .x = 0, .y = 0 })){
                    if (foundValidExt) {
                        if(diffCtx->loadedTextureHandle != GFX_POOL_GFX_TEXTURE_COUNT_MAX){
                            diffCtx->deferRelease.active = true;
                            diffCtx->deferRelease.counter = KAH_SWAP_CHAIN_IMAGE_COUNT;
                            diffCtx->deferRelease.loadedTextureHandle = diffCtx->loadedTextureHandle;
                            diffCtx->deferRelease.imguiImageId = diffCtx->imguiImageId;
                            diffCtx->loadedTextureHandle = GFX_POOL_GFX_TEXTURE_COUNT_MAX;
                        }
                        diffCtx->selectedFileIndex = (int)i;
                        char texturePath[KAH_FILESYSTEM_MAX_PATH] = {};
                        sprintf(texturePath, "%s\\%s",diffCtx->dirPath, fileName);
                        diffCtx->loadedTextureHandle = gfx_texture_load_from_file(texturePath);

                        VkSampler sampler = gfx_get_sampler_linear();
                        GfxTexture* tex = gfx_pool_gfx_texture_get(diffCtx->loadedTextureHandle);
                        VkDescriptorSet set = cImGui_ImplVulkan_AddTexture(sampler,tex->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
                        diffCtx->imguiImageId._TexID = (ImTextureID)set;
                    }
                }
                ImGui_PopStyleColor();
            }
            ImGui_EndTable();
            if (diffCtx->loadedTextureHandle != GFX_POOL_GFX_TEXTURE_COUNT_MAX) {
                ImGui_Image(diffCtx->imguiImageId, (ImVec2) { .x = 128, .y = 128 });
            }
        }
    }
}

static void widget_image_diff_defer_release(DiffSelectionCtx* diffCtx) {
    //TODO: replace counter with GFX frame index i.e. defer release of images after 1 swapchains worth of frames have passed.
    if (diffCtx->deferRelease.active) {
        if (diffCtx->deferRelease.counter == 0) {
            core_assert(diffCtx->deferRelease.loadedTextureHandle != GFX_POOL_GFX_TEXTURE_COUNT_MAX);
            diffCtx->deferRelease.active = false;
            cImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)diffCtx->deferRelease.imguiImageId._TexID);
            gfx_texture_cleanup(diffCtx->deferRelease.loadedTextureHandle);
            diffCtx->deferRelease.loadedTextureHandle = GFX_POOL_GFX_TEXTURE_COUNT_MAX;
        }
        else {
            diffCtx->deferRelease.counter--;
        }
    }
}

static void widget_image_differ_update(){
    WidgetInfo* cvarWidget = &s_widgetTable[KAH_WIDGET_IMAGE_DIFF];
    if (cvarWidget->isActive) {
        if (ImGui_Begin(cvarWidget->name, &cvarWidget->isActive, ImGuiWindowFlags_None)) {
            if (ImGui_BeginTable("DiffTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui_TableNextColumn();
                widget_image_diff_selection(&s_diffImageCtxA);

                ImGui_TableNextColumn();
                widget_image_diff_selection(&s_diffImageCtxB);

                ImGui_EndTable();
            }
            ImGui_End();
        }
    }

    widget_image_diff_defer_release(&s_diffImageCtxA);
    widget_image_diff_defer_release(&s_diffImageCtxB);
}

static void widget_image_differ_create() {
    sprintf(s_diffImageCtxA.dirPath, "%s/assets/textures/", fs_exe_dir());
    s_diffImageCtxA.loadedTextureHandle = GFX_POOL_GFX_TEXTURE_COUNT_MAX;
    sprintf(s_diffImageCtxB.dirPath, "%s/assets/textures/", fs_exe_dir());
    s_diffImageCtxB.loadedTextureHandle = GFX_POOL_GFX_TEXTURE_COUNT_MAX;
}

static void widget_image_differ_cleanup() {
    if (s_diffImageCtxA.fileNames != NULL) {
        allocators()->cstd.free(s_diffImageCtxA.fileNames);
    }
    if (s_diffImageCtxB.fileNames != NULL) {
        allocators()->cstd.free(s_diffImageCtxB.fileNames);
    }
}

static void widget_imgui_demo_update() {
    WidgetInfo* imguiDemoWidget = &s_widgetTable[KAH_WIDGET_IMGUI_DEMO_MENU];
    if (imguiDemoWidget->isActive) {
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
        if(gfx_has_drawable_surface()){
            gfx_imgui_demo_window(); //Client side imgui code must run based on has gfx_has_drawable_surface test.
        }
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)
    }
}
//=============================================================================

//===API=======================================================================
void widget_manager_update(){
    widget_toolbar_update();
    widget_state_update();
    widget_cvar_update();
    widget_image_differ_update();
    widget_imgui_demo_update();
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void widget_manager_create(){
    memset(s_widgetTable, 0, sizeof(s_widgetTable));
    s_widgetTable[KAH_WIDGET_ACTIVE_WIDGETS_MENU]   = (WidgetInfo){ .isActive = false,  .name = "Active Widgets Menu",   .toolbarTabName = "Editor"};
    s_widgetTable[KAH_WIDGET_TOOLBAR_NAV_MENU]      = (WidgetInfo){ .isActive = true,   .name = "Navigation bar"},
    s_widgetTable[KAH_WIDGET_CVARS]                 = (WidgetInfo){ .isActive = false,  .name = "Console Variabe debug", .toolbarTabName = "Debug Tools",    .shortcut = "`"};
    s_widgetTable[KAH_WIDGET_IMAGE_DIFF]            = (WidgetInfo){ .isActive = false,  .name = "Image differ",          .toolbarTabName = "Debug Tools"};
    s_widgetTable[KAH_WIDGET_IMGUI_DEMO_MENU]       = (WidgetInfo){ .isActive = true,   .name = "Imgui Demo Menu",       .toolbarTabName = "Debug Tools" };

    widget_image_differ_create();
}

void widget_manager_cleanup(){
    memset(s_widgetTable, 0, sizeof(s_widgetTable));

    widget_image_differ_cleanup();
}
//=============================================================================

#endif //WINDGET_MANAGER_C_H
