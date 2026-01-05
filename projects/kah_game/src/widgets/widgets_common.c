//===INCLUDES==================================================================
#include <client/widgets/widgets_common.h>

#include <dcimgui.h>
#include <dcimgui_internal.h>
//=============================================================================

//===EXTERNAL_STRUCTS==========================================================
extern ImGuiContext* GImGui;  // Current implicit context pointer // from #include <dcimgui_internal.h>
//=============================================================================

//===API=======================================================================
bool draw_vec3_control( const char *label, vec3f *values, float resetValue, float columnWidth, float minDrag) {
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

bool widget_draw_camera(Camera *camera) {
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

bool widget_draw_transform(Transform *transform) {
    bool outEdited = false;
    if (ImGui_CollapsingHeader("Transform",0)) {
        outEdited |= draw_vec3_control("Pos", &transform->position, 0.0f, 35.0f, 0.1f);
        outEdited |= draw_vec3_control("Rot", &transform->rotation, 0.0f, 35.0f, 0.1f);
        outEdited |= draw_vec3_control("Scl", &transform->scale, 0.0f, 35.0f, 0.1f);
    }
    return outEdited;
}
//=============================================================================