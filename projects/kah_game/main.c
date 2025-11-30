//===INCLUDES==================================================================
#include <kah_core/defines.h>
#include <kah_core/allocators.h>
#include <kah_core/assert.h>
#include <kah_core/memory.h>
#include <kah_core/fixed_array.h>
#include <kah_core/input.h>
#include <kah_core/time.h>
#include <kah_core/window.h>
#include <kah_core/bit_array.h>
#include <kah_core/cvar.h>
#include <kah_core/core_cvars.h>
#include <kah_core/filesystem.h>

#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_imgui.h>

#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
#include <dcimgui.h>
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)

#include <stdint.h>
#include <string.h>
//=============================================================================

#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
static void client_cvar_debug_vec2i(const char* varName, vec2i_cvar_t* cvar) {
    bool showCvarWindow = true;
    ImGui_Begin("cvar debug display", &showCvarWindow, ImGuiWindowFlags_None);
    {
        if(ImGui_CollapsingHeader(varName, ImGuiTreeNodeFlags_None))
        {
            ImGui_Text("min (%i , %i)", cvar->min.x, cvar->min.y);
            ImGui_Text("max (%i , %i)", cvar->max.x, cvar->max.y);
            char textBuf[128] = {};
            sprintf_s(textBuf, 128, "##cvar_current_x_%s", varName);
            ImGui_DragIntEx(textBuf, &cvar->current.x, 1, cvar->min.x, cvar->max.x, "%i", ImGuiSliderFlags_AlwaysClamp);
            sprintf_s(textBuf, 128, "##cvar_current_y_%s", varName);
            ImGui_DragIntEx(textBuf, &cvar->current.y, 1, cvar->min.y, cvar->max.y, "%i", ImGuiSliderFlags_AlwaysClamp);
        }
    }
    ImGui_End();
}

void imgui_update() {
    gfx_imgui_begin();
    if(gfx_has_drawable_surface()){
        gfx_imgui_demo_window();
        //Client side imgui code must run based on has gfx_has_drawable_surface test.
    }
}
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)

int main(int argc, char** argv)
{
    fs_create(argc, argv);
    mem_create();
    allocator_create();
    {
        cvar_create("options.csv");
        core_cvars_register();
        window_create("kah engine - runtime", vec2i_cvar_get(g_coreCvars.windowSize), vec2i_cvar_get(g_coreCvars.windowPosition));
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
        window_set_procedure_callback_func(gfx_imgui_get_proc_function_pointer());
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)
        time_create();
        input_create();
        gfx_create(window_get_handle());

        while (window_is_open()){
            mem_arena_reset();
            time_tick();
            input_set_time(time_current());
            window_update();
            input_update();
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
            imgui_update();
            client_cvar_debug_vec2i("windowSize", g_coreCvars.windowSize);
            client_cvar_debug_vec2i("windowPosition", g_coreCvars.windowPosition);
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)
            gfx_update();
        }

        gfx_cleanup();
        input_cleanup();
        time_cleanup();
        window_cleanup();
        cvar_cleanup();
    }
    allocator_cleanup();
    mem_cleanup();
    fs_cleanup();

    mem_dump_info();
    core_assert_msg(mem_alloc_table_empty(), "err: memory leaks");
    if (mem_alloc_table_empty()) {
        printf("leaks");
    }
    return 0;
}