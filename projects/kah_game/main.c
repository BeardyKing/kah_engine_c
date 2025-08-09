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

#include <kah_gfx/gfx_interface.h>

#include <stdio.h>
#include <stdint.h>
//=============================================================================

void run_u64_util_tests() {
    // BitArray_64 array = {
    //     .header.bitCount = 64,
    //     .buf = { 0 }
    // };
    BitArrayDynamic array = {
        .header.bitCount = 64,
        .buf = allocators()->arena.alloc(sizeof(uint64_t))->bufferAddress
    };
    uint64_t* word = bitarray_buffer(&array.header);

    u64_set_bit(word, 3);
    printf("Set bit 3:\n");
    bitarray_print(&array.header);
    core_assert(u64_is_bit_set(*word, 3));

    u64_clear_bit(word, 3);
    printf("Cleared bit 3:\n");
    bitarray_print(&array.header);
    core_assert(!u64_is_bit_set(*word, 3));

    u64_toggle_bit(word, 7);
    printf("Toggled bit 7 (set):\n");
    bitarray_print(&array.header);
    core_assert(u64_is_bit_set(*word, 7));

    u64_toggle_bit(word, 7);
    printf("Toggled bit 7 (cleared):\n");
    bitarray_print(&array.header);
    core_assert(!u64_is_bit_set(*word, 7));

    u64_set_bit_range(word, 4, 8);
    printf("Set bit range [4,8):\n");
    bitarray_print(&array.header);
    core_assert((*word & 0xF0) == 0xF0);

    u64_clear_bit_range(word, 5, 7);
    printf("Cleared bit range [5,7):\n");
    bitarray_print(&array.header);
    uint64_t expected = (1ULL << 7) | (1ULL << 4);
    core_assert((*word & 0xF0) == expected);

    size_t setBits = u64_count_set_bits(*word);
    printf("Counted %zu set bits:\n", setBits);
    bitarray_print(&array.header);
    core_assert(setBits == 2);

    size_t unsetBits = u64_count_unset_bits(*word);
    printf("Counted %zu unset bits:\n", unsetBits);
    bitarray_print(&array.header);
    core_assert(unsetBits == 62);

    *word = 0x0000000000001000ULL;
    printf("Bit pattern with only bit 12 set:\n");
    bitarray_print(&array.header);
    size_t lz = u64_count_leading_zeros(*word);
    size_t tz = u64_count_trailing_zeros(*word);
    printf("Leading zeros: %zu, Trailing zeros: %zu\n", lz, tz);
    core_assert(lz == 51);
    core_assert(tz == 12);

    printf("=== All u64 util tests passed ===\n");
}

int main(void)
{
    const vec2i windowSize = {1024, 768};
    mem_create();
    allocator_create();

    run_u64_util_tests();

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

        {
            uint64_t u64Value = INT64_MAX; // + 1;
            int64_t i64Value = truncate_cast(int64_t, u64Value);

            printf("%lld %llu \n", i64Value, i64Value);
            printf("%lld %llu \n", type_max(i64Value), type_max(u64Value));
            printf("%lld %u \n", type_min(i64Value), type_min(u64Value));
        }
        {
            FixedArray intArr = fixed_array_create(allocators()->cstd, sizeof(uint32_t), 10);
            void* buf = fixed_array_get(&intArr, 0);
            printf("%p - %p\n", buf, intArr.info->bufferAddress);
            printf("%u - %u\n", *(uint32_t*)buf, *(uint32_t*)intArr.info->bufferAddress);
            for (uint32_t i = 0; i < 10; ++i){
                fixed_array_insert(&intArr, i, &i);
            }
            for (uint32_t i = 0; i < 10/* + 1*/; ++i){
                uint32_t* data = fixed_array_get(&intArr, i);
                core_assert(data == &((uint32_t*)buf)[i]);
                printf("%u,", *data);
            }
            printf("\n");
            fixed_array_cleanup(allocators()->cstd, &intArr);
        }
        {
            AllocInfo* alloc = allocators()->cstd.alloc(sizeof(int32_t));
            int32_t* arr = (int32_t*)alloc->bufferAddress;
            *arr = 32;
            printf("%i\n", *(uint32_t*)alloc->bufferAddress);

            allocators()->cstd.realloc(alloc, sizeof(int32_t) * 10);
            for (int32_t i = 0; i < 10; ++i){
                if (i > 0){
                    arr[i] = i;
                }
                printf("%i,", arr[i]);
            }
            printf("\n");

            allocators()->cstd.free(alloc);
        }
    }
    allocator_cleanup();
    mem_cleanup();

    mem_dump_info();
    core_assert_msg(mem_alloc_table_empty(), "err: memory leaks");
    return 0;
}