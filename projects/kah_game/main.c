//===INCLUDES==================================================================
#include <client/widgets/widget_manager.h>

#include <kah_core/defines.h>
#include <kah_core/allocators.h>
#include <kah_core/assert.h>
#include <kah_core/memory.h>
#include <kah_core/input.h>
#include <kah_core/time.h>
#include <kah_core/window.h>
#include <kah_core/bit_array.h>
#include <kah_core/cvar.h>
#include <kah_core/core_cvars.h>
#include <kah_core/filesystem.h>

#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_imgui.h>

#include <kah_math/print.h>
#include <kah_math/transform.h>
//=============================================================================

int main(int argc, char** argv){
    // Transform t = {};
    // t.position = (vec3f){8,4,32};
    // t.rotation = (vec3f){45,1,0};
    // t.scale = (vec3f){1,2,3};
    // // mat4f mm = transform_model_matrix(&t);
    // mat4f mm = transform_model_matrix_quat_cast(&t);
    // print_mat4f(&mm);
    //
    // return -1;
    fs_create(argc, argv);
    mem_create();
    allocator_create();
    {
        cvar_create("options.csv");
        core_cvars_register();
        widget_manager_create();
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
            gfx_imgui_begin();
            widget_manager_update();
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)
            gfx_update();
        }

        gfx_cleanup();
        input_cleanup();
        time_cleanup();
        window_cleanup();
        widget_manager_cleanup();
        cvar_cleanup();
    }
    allocator_cleanup();
    mem_cleanup();
    fs_cleanup();

    mem_dump_info();
    core_assert_msg(mem_alloc_table_empty(), "err: memory leaks");
    if (!mem_alloc_table_empty()) {
        printf("leaks");
    }
    return 0;
}