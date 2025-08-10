//===INCLUDES==================================================================
#include <kah_core/defines.h>
#include <kah_core/allocators.h>
#include <kah_core/assert.h>
#include <kah_core/memory.h>
#include <kah_core/fixed_array.h>
#include <kah_core/input.h>
#include <kah_core/time.h>
#include <kah_core/window.h>
#include <kah_core/utils.h>
#include <kah_core/bit_array.h>

#if CHECK_FEATURE(FEATURE_RUN_TESTS)
#include <kah_core/tests/test_bits.h>
#include <kah_core/tests/test_data_structures.h>
#endif

#include <kah_gfx/gfx_interface.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
//=============================================================================

void test_run_all(){
#if CHECK_FEATURE(FEATURE_RUN_TESTS)
    //TODO: setup project that just runs tests for various libs
    mem_create();
    allocator_create();
    {

        test_run_bitset();
        test_run_bitarray();

        test_run_data_structures();
    }
    mem_cleanup();
    allocator_cleanup();
    core_assert_msg(mem_alloc_table_empty(), "err: memory leaks");
#endif
}

int main(void)
{
    test_run_all();

    const vec2i windowSize = {1024, 768};
    mem_create();
    allocator_create();

    {
        window_create("kah engine - runtime", windowSize, KAH_WINDOW_POSITION_CENTERED);
        time_create();
        input_create();
        gfx_create(window_get_handle());

        while (window_is_open()){
            time_tick();
            input_set_time(time_current());
            window_update();
            input_update();
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