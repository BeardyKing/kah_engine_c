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

#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_imgui.h>

#include <stdint.h>
#include <string.h>
//=============================================================================


#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
void imgui_update() {
    gfx_imgui_begin();
    if(gfx_has_drawable_surface()){
        gfx_imgui_demo_window();
        //Client side imgui code must run based on has gfx_has_drawable_surface test.
    }
}
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)

int main(void)
{
    mem_create();
    allocator_create();
    {
        window_create("kah engine - runtime", (vec2i){1024, 768}, KAH_WINDOW_POSITION_CENTERED);
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
    }
    allocator_cleanup();
    mem_cleanup();

    mem_dump_info();
    core_assert_msg(mem_alloc_table_empty(), "err: memory leaks");
    return 0;
}