//===INCLUDES==================================================================
#include <client/widgets/widget_pool_inspector.h>

#include <kah_gfx/gfx_pool.h>
#include <kah_gfx/vulkan/gfx_vulkan_imgui.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>

#include <kah_core/assert.h>

#include <dcimgui.h>
#include <dcimgui_internal.h>

#include <stdint.h>
#include <stdio.h>
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern ImGuiContext* GImGui;  // Current implicit context pointer // from #include <dcimgui_internal.h>
//=============================================================================
enum SelectedPool {
    WIDGET_SELECTED_POOL_NONE = 0,
    WIDGET_SELECTED_POOL_CAMERA_ENT = 1,
    WIDGET_SELECTED_POOL_LIT_ENT = 2,
} typedef SelectedPool;

//===INTERNAL_STRUCTS==========================================================
struct PoolWidget{
    SelectedPool selectedPool;
    int64_t poolIndex;

    bool selectorEnabled;
    bool inspectorEnabled;
} typedef PoolWidget;

static PoolWidget s_poolWidget;
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static void pool_widget_reset(PoolWidget* widget){
    core_assert(widget != nullptr);
    *widget = (PoolWidget){
        .selectedPool = WIDGET_SELECTED_POOL_NONE,
        .poolIndex = -1,

        .selectorEnabled = true,
        .inspectorEnabled = true,
    };
}

static bool draw_vec3_control( const char *label, vec3f *values, float resetValue, float columnWidth, float minDrag) {
    bool outEdited = false;

    ImGuiIO *io = ImGui_GetIO();
    ImFont *boldFont = io->Fonts->Fonts.Data[0];

    ImGui_PushID(label);

    ImGui_ColumnsEx(2, NULL, false);
    ImGui_SetColumnWidth(0, columnWidth);
    ImGui_Text("%s", label);
    ImGui_NextColumn();

    ImGui_PushMultiItemsWidths(3, ImGui_CalcItemWidth());
    ImGui_PushStyleVarImVec2(ImGuiStyleVar_ItemSpacing, (ImVec2){0.0f, 0.0f});
    float lineHeight = GImGui->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    //===Y=====================================================================
    ImGui_PushStyleColorImVec4(ImGuiCol_Button,        (ImVec4){0.8f, 0.1f, 0.15f, 1.0f});
    ImGui_PushStyleColorImVec4(ImGuiCol_ButtonHovered, (ImVec4){0.9f, 0.2f, 0.2f,  1.0f});
    ImGui_PushStyleColorImVec4(ImGuiCol_ButtonActive,  (ImVec4){0.8f, 0.1f, 0.15f, 1.0f});
    ImGui_PushFont(boldFont);
    if (ImGui_ButtonEx("X", buttonSize)){
        values->x = resetValue;
    }
    ImGui_PopFont();
    ImGui_PopStyleColorEx(3);

    ImGui_SameLineEx(0.0f, 0.0f);
    outEdited |= ImGui_DragFloatEx("##X", &values->x, minDrag, 0.0f, 0.0f, "%.3f", 0);
    ImGui_PopItemWidth();
    //=========================================================================
    ImGui_SameLineEx(0.0f, 0.0f);
    //===Y=====================================================================
    ImGui_PushStyleColorImVec4(ImGuiCol_Button,        (ImVec4){0.2f, 0.7f, 0.2f, 1.0f});
    ImGui_PushStyleColorImVec4(ImGuiCol_ButtonHovered, (ImVec4){0.3f, 0.8f, 0.3f, 1.0f});
    ImGui_PushStyleColorImVec4(ImGuiCol_ButtonActive,  (ImVec4){0.2f, 0.7f, 0.2f, 1.0f});
    ImGui_PushFont(boldFont);
    if (ImGui_ButtonEx("Y", buttonSize)){
        values->y = resetValue;
    }
    ImGui_PopFont();
    ImGui_PopStyleColorEx(3);

    ImGui_SameLineEx(0.0f, 0.0f);
    outEdited |= ImGui_DragFloatEx("##Y", &values->y, minDrag, 0.0f, 0.0f, "%.3f", 0);
    ImGui_PopItemWidth();
    //=========================================================================
    ImGui_SameLineEx(0.0f, 0.0f);
    //===Z=====================================================================
    ImGui_PushStyleColorImVec4(ImGuiCol_Button,        (ImVec4){0.1f, 0.25f, 0.8f, 1.0f});
    ImGui_PushStyleColorImVec4(ImGuiCol_ButtonHovered, (ImVec4){0.2f, 0.35f, 0.9f, 1.0f});
    ImGui_PushStyleColorImVec4(ImGuiCol_ButtonActive,  (ImVec4){0.1f, 0.25f, 0.8f, 1.0f});
    ImGui_PushFont(boldFont);
    if (ImGui_ButtonEx("Z", buttonSize)){
        values->z = resetValue;
    }
    ImGui_PopFont();
    ImGui_PopStyleColorEx(3);

    ImGui_SameLineEx(0.0f, 0.0f);
    outEdited |= ImGui_DragFloatEx("##Z", &values->z, minDrag, 0.0f, 0.0f, "%.3f", 0);
    ImGui_PopItemWidth();
    //=========================================================================
    ImGui_PopStyleVar();
    ImGui_ColumnsEx(1, NULL, false);
    ImGui_PopID();

    return outEdited;
}

static bool widget_draw_camera(Camera *camera) {
    constexpr float FOV_DRAG_AMOUNT = 0.5f;
    constexpr float Z_DRAG_AMOUNT   = 1.0f;
    constexpr float Z_MIN           = 0.1f;
    constexpr float Z_MAX           = 100000.0f;

    bool outEdited = false;

    if (ImGui_CollapsingHeader("Camera", 0)) {
        ImGui_Text("fov   ");
        ImGui_SameLineEx(0.0f, 0.0f);
        outEdited |= ImGui_DragFloatEx( "##fov", &camera->fov, FOV_DRAG_AMOUNT, 0.0f, 0.0f, "%.3f", 0);
        camera->fov = clamp_f32(camera->fov, 0.1f, 179.9f);

        ImGui_Text("zNear ");
        ImGui_SameLineEx(0.0f, 0.0f);
        outEdited |= ImGui_DragFloatEx( "##zNear", &camera->zNear, Z_DRAG_AMOUNT, 0.0f, 0.0f, "%.1f", 0);
        camera->zNear = clamp_f32(camera->zNear, Z_MIN, Z_MAX);

        ImGui_Text("zFar  ");
        ImGui_SameLineEx(0.0f, 0.0f);
        outEdited |= ImGui_DragFloatEx( "##zFar", &camera->zFar, Z_DRAG_AMOUNT, 0.0f, 0.0f, "%.1f", 0 );
        camera->zFar = clamp_f32(camera->zFar, Z_MIN, Z_MAX);
    }
    return outEdited;
}

static bool widget_draw_transform(Transform *transform) {
    bool outEdited = false;
    if (ImGui_CollapsingHeader("Transform",0)) {
        outEdited |= draw_vec3_control("Pos", &transform->position, 0.0f, 35.0f, 0.1f);
        outEdited |= draw_vec3_control("Rot", &transform->rotation, 0.0f, 35.0f, 0.1f);
        outEdited |= draw_vec3_control("Scl", &transform->scale, 0.0f, 35.0f, 0.1f);
    }
    return outEdited;
}

static void widget_select_pool_and_pool_index()
{
    char strBuf[256];

    ImGui_Begin("Pool Selector", &s_poolWidget.selectorEnabled, 0);
    {
        if (ImGui_CollapsingHeader("Pool: camera entities", 0)){
            CameraEntityHandle entId = GFX_POOL_NULL_HANDLE;
            while ((entId = gfx_pool_camera_entity_get_next_active(entId)) != GFX_POOL_NULL_HANDLE){
                CameraEntity* ent = gfx_pool_camera_entity_get(entId);
                sprintf(strBuf, "Name: \"%s\" - %u", ent->debug_name, entId);
                bool isSelected = s_poolWidget.poolIndex == entId && s_poolWidget.selectedPool == WIDGET_SELECTED_POOL_CAMERA_ENT;
                if(ImGui_SelectableEx(strBuf, isSelected, ImGuiSelectableFlags_None, (ImVec2){0,0})){
                    s_poolWidget.poolIndex = entId;
                    s_poolWidget.selectedPool = WIDGET_SELECTED_POOL_CAMERA_ENT;
                }
            }
        }

        if (ImGui_CollapsingHeader("Pool: lit entities", 0)){
            LitEntityHandle entId = GFX_POOL_NULL_HANDLE;
            while ((entId = gfx_pool_lit_entity_get_next_active(entId)) != GFX_POOL_NULL_HANDLE){
                LitEntity* ent = gfx_pool_lit_entity_get(entId);
                sprintf(strBuf, "Name: \"%s\" - %u", ent->debug_name, entId);
                bool isSelected = s_poolWidget.poolIndex == entId && s_poolWidget.selectedPool == WIDGET_SELECTED_POOL_LIT_ENT;
                if(ImGui_SelectableEx(strBuf, isSelected, ImGuiSelectableFlags_None, (ImVec2){0,0})){
                    s_poolWidget.poolIndex = entId;
                    s_poolWidget.selectedPool = WIDGET_SELECTED_POOL_LIT_ENT;
                }
            }
        }

    }
    ImGui_End();
}

static bool widget_pool_debug_name(char* debugName, uint32_t count){
    bool outEdited = false;
    ImGui_SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    ImGui_Text("debug name  ");
    ImGui_SameLineEx(0.0f, 0.0f);
    outEdited = ImGui_InputTextEx( "##Name", debugName, count, ImGuiInputTextFlags_AutoSelectAll, NULL, NULL );
    return outEdited;
}

static void widget_pool_inspector_camera_entity(CameraEntity* camEntity){
    Transform* transform = gfx_pool_transform_get(camEntity->transformIndex);
    Camera* cam = gfx_pool_camera_get(camEntity->cameraIndex);

#if KAH_DEBUG
    ImGui_SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    widget_pool_debug_name(camEntity->debug_name, 128);
#endif //#if KAH_DEBUG

    ImGui_SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    widget_draw_transform(transform);

    ImGui_SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    widget_draw_camera(cam);
}

void widget_pool_inspector_lit_entity(LitEntity* litEntity){
    Transform* transform = gfx_pool_transform_get(litEntity->transformIndex);

#if KAH_DEBUG
    ImGui_SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    widget_pool_debug_name(litEntity->debug_name, 128);
#endif //#if KAH_DEBUG

    ImGui_SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    widget_draw_transform(transform);

    //TODO: allow for changing mesh/material and also creating / loading a new mesh / material.
    ImGui_Text("Mesh Index %u", litEntity->meshIndex);
    ImGui_Text("Material Index %u", litEntity->materialIndex);
}

static void widget_pool_inspector(){
    ImGui_Begin("Pool Inspector", &s_poolWidget.inspectorEnabled, 0);
    {
        switch (s_poolWidget.selectedPool){
        case WIDGET_SELECTED_POOL_CAMERA_ENT:
            CameraEntity* camEntity = gfx_pool_camera_entity_get(s_poolWidget.poolIndex);
            widget_pool_inspector_camera_entity(camEntity);
            break;
        case WIDGET_SELECTED_POOL_LIT_ENT:
            LitEntity* litEntity = gfx_pool_lit_entity_get(s_poolWidget.poolIndex);
            widget_pool_inspector_lit_entity(litEntity);
            break;
        case WIDGET_SELECTED_POOL_NONE:
            break;
        }
    }
    ImGui_End();
}
//=============================================================================

//===API=======================================================================
void widget_pool_inspector_update(){
    widget_select_pool_and_pool_index();
    widget_pool_inspector();
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void widget_pool_inspector_create(){
    pool_widget_reset(&s_poolWidget);
}

void widget_pool_inspector_cleanup(){
    pool_widget_reset(&s_poolWidget);
}
//=============================================================================
