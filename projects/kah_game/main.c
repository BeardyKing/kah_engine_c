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
#include <kah_core/filesystem.h>

#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_imgui.h>

#include <stdint.h>
#include <string.h>
//=============================================================================

i32_cvar_t* core_windowX = nullptr;
i32_cvar_t* core_windowY = nullptr;
bool_cvar_t* core_vSyncEnabled = nullptr;
u32_cvar_t* core_u32Test = nullptr;
f32_cvar_t* core_f32Test = nullptr;
vec2i_cvar_t* core_vec2iTest = nullptr;

void cvar_register(){
    core_windowX = i32_cvar_create("windowSizeX", C_VAR_DISK, 1024, 128, INT32_MAX);
    core_windowY = i32_cvar_create("windowSizeY", C_VAR_DISK, 768, 128, INT32_MAX);
    core_vSyncEnabled = bool_cvar_create("vSyncEnabled", C_VAR_DISK, true); 
    core_u32Test = u32_cvar_create("cvarTestU32", C_VAR_DISK, UINT32_MAX, 0, UINT32_MAX);
    core_f32Test = f32_cvar_create("cvarTestF32", C_VAR_DISK, 14.125, 0, 1000.0f);
    core_vec2iTest = vec2i_cvar_create("cvarTestVec2i32", C_VAR_DISK, (vec2i) { .x = 1024, .y = 768 }, (vec2i) { .x = 0, .y = 0 }, (vec2i) { .x = INT32_MAX, .y = INT32_MAX });

    printf("%u\n", u32_cvar_get(core_u32Test));
    printf("%f\n", f32_cvar_get(core_f32Test));
    printf("%i , %i\n", vec2i_cvar_get(core_vec2iTest).x, vec2i_cvar_get(core_vec2iTest).y);
}

#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
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
        cvar_register();
        window_create("kah engine - runtime", (vec2i){ .x = i32_cvar_get(core_windowX), .y = i32_cvar_get(core_windowY) }, KAH_WINDOW_POSITION_CENTERED);
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