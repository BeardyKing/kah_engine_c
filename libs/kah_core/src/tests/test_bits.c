#include <kah_core/defines.h>
#if CHECK_FEATURE(FEATURE_RUN_TESTS)

//===INCLUDES==================================================================
#include <kah_core/tests/test_bits.h>
#include <kah_core/bit_array.h>

#include <string.h>
//=============================================================================

//===INTERNAL==================================================================
static void internal_run_bitarray_util_tests(){
    BitArray_256 bitarray = {
        .header.bitCount = 256,
        .buf = {0ULL,0ULL,0ULL,0ULL},
    };

    uint64_t* buf = bitarray_buffer(&bitarray.header);

    printf("=== Setting all bits ===\n");
    bitarray_set_bit_range(&bitarray.header, 0, 256);
    bitarray_print(&bitarray.header);
    core_assert(bitarray_count_set_bits(&bitarray.header) == 256);

    printf("=== Clearing all bits ===\n");
    bitarray_clear_bit_range(&bitarray.header, 0, 256);
    bitarray_print(&bitarray.header);
    core_assert(bitarray_count_set_bits(&bitarray.header) == 0);

    printf("=== Checking all bits are initially zero ===\n");
    bitarray_print(&bitarray.header);
    core_assert(bitarray_count_set_bits(&bitarray.header) == 0);
    core_assert(bitarray_count_unset_bits(&bitarray.header) == 256);
    core_assert(bitarray_count_trailing_zeros(&bitarray.header) == 256);
    core_assert(bitarray_count_leading_zeros(&bitarray.header) == 256);

    printf("=== Setting a single bit at index 5 ===\n");
    bitarray_set_bit(&bitarray.header, 5);
    bitarray_print(&bitarray.header);
    core_assert(bitarray_is_bit_set(&bitarray.header, 5));
    core_assert(bitarray_count_set_bits(&bitarray.header) == 1);
    core_assert(bitarray_count_unset_bits(&bitarray.header) == 255);
    core_assert(bitarray_count_trailing_zeros(&bitarray.header) == 5);
    core_assert(bitarray_count_leading_zeros(&bitarray.header) == 250);

    printf("=== Clearing the bit at index 5 ===\n");
    bitarray_clear_bit(&bitarray.header, 5);
    bitarray_print(&bitarray.header);
    core_assert(!bitarray_is_bit_set(&bitarray.header, 5));
    core_assert(bitarray_count_set_bits(&bitarray.header) == 0);
    core_assert(bitarray_count_trailing_zeros(&bitarray.header) == 256);

    printf("=== Toggling bit 100 on and off ===\n");
    bitarray_toggle_bit(&bitarray.header, 100);
    bitarray_print(&bitarray.header);
    core_assert(bitarray_is_bit_set(&bitarray.header, 100));
    bitarray_toggle_bit(&bitarray.header, 100);
    bitarray_print(&bitarray.header);
    core_assert(!bitarray_is_bit_set(&bitarray.header, 100));

    printf("=== Setting a bit range [64..128] ===\n");
    bitarray_set_bit_range(&bitarray.header, 64, 128);
    bitarray_print(&bitarray.header);
    for (size_t i = 64; i < 128; ++i)
    {
        core_assert(bitarray_is_bit_set(&bitarray.header, i));
    }
    core_assert(bitarray_count_set_bits(&bitarray.header) == 64);

    printf("=== Clearing a bit range [80..96] ===\n");
    bitarray_clear_bit_range(&bitarray.header, 80, 96);
    bitarray_print(&bitarray.header);
    for (size_t i = 80; i < 96; ++i)
    {
        core_assert(!bitarray_is_bit_set(&bitarray.header, i));
    }
    core_assert(bitarray_count_set_bits(&bitarray.header) == 48);

    printf("=== Verifying leading and trailing zeros after specific sets ===\n");
    bitarray_set_bit(&bitarray.header, 0);
    bitarray_print(&bitarray.header);
    core_assert(bitarray_count_trailing_zeros(&bitarray.header) == 0);

    memset(buf, 0, sizeof(bitarray.buf));
    bitarray_set_bit(&bitarray.header, 255);
    bitarray_print(&bitarray.header);
    core_assert(bitarray_count_leading_zeros(&bitarray.header) == 0);
    core_assert(bitarray_count_trailing_zeros(&bitarray.header) == 255);

    printf("=== All bitarray util tests passed ===\n");
}

static void internal_test_bitarray_ranges(){
    BitArray_256 bitarray = {
        .header.bitCount = 256,
        .buf = {0ULL,0ULL,0ULL,0ULL},
    };

    printf("=== Setting all bits ===\n");
    bitarray_set_bit_range(&bitarray.header, 0, 256);
    bitarray_print(&bitarray.header);
    for (size_t i = 0; i < 256; ++i){
        core_assert(bitarray_is_bit_set(&bitarray.header, i));
    }

    printf("=== Clearing all bits ===\n");
    bitarray_clear_bit_range(&bitarray.header, 0, 256);
    bitarray_print(&bitarray.header);

    printf("=== Performing a no-op with zero-length range ===\n");
    bitarray_set_bit_range(&bitarray.header, 10, 10);
    bitarray_print(&bitarray.header);

    printf("=== Setting bits within a single word [8..16] ===\n");
    bitarray_set_bit_range(&bitarray.header, 8, 16);
    bitarray_print(&bitarray.header);

    printf("=== Clearing a subrange [10..12] ===\n");
    bitarray_clear_bit_range(&bitarray.header, 10, 12);
    bitarray_print(&bitarray.header);

    printf("=== Setting bits across word boundaries [60..68] ===\n");
    bitarray_set_bit_range(&bitarray.header, 60, 68);
    bitarray_print(&bitarray.header);

    printf("=== Clearing a large middle range [80..160] after setting [64..192] ===\n");
    bitarray_clear_bit_range(&bitarray.header, 0, 256);
    bitarray_set_bit_range(&bitarray.header, 64, 192);
    bitarray_clear_bit_range(&bitarray.header, 80, 160);
    bitarray_print(&bitarray.header);

    printf("=== Resetting and clearing entire bitarray again ===\n");
    bitarray_set_bit_range(&bitarray.header, 0, 256);
    bitarray_print(&bitarray.header);
    bitarray_clear_bit_range(&bitarray.header, 0, 256);
    bitarray_print(&bitarray.header);

    printf("=== Setting and clearing non-word-aligned ranges ===\n");
    bitarray_set_bit_range(&bitarray.header, 3, 70);
    bitarray_clear_bit_range(&bitarray.header, 5, 65);
    bitarray_print(&bitarray.header);

    printf("=== All bitarray range tests passed ===\n");
}

static void internal_run_u64_util_tests(){
    uint64_t bitsetBackingBuffer = 0;
    uint64_t* word = &bitsetBackingBuffer;
    BitArrayDynamic array = (BitArrayDynamic){.header.bitCount = 64, .buf = word};

    printf("=== Setting bit 3 ===\n");
    u64_set_bit(word, 3);
    bitarray_print(&array.header);

    printf("=== Clearing bit 3 ===\n");
    u64_clear_bit(word, 3);
    bitarray_print(&array.header);

    printf("=== Toggling bit 7 on ===\n");
    u64_toggle_bit(word, 7);
    bitarray_print(&array.header);

    printf("=== Toggling bit 7 off ===\n");
    u64_toggle_bit(word, 7);
    bitarray_print(&array.header);

    printf("=== Setting bit range [4..8] ===\n");
    u64_set_bit_range(word, 4, 8);
    bitarray_print(&array.header);

    printf("=== Clearing bit range [5..7] ===\n");
    u64_clear_bit_range(word, 5, 7);
    bitarray_print(&array.header);

    printf("=== Counting set bits ===\n");
    size_t setBits = u64_count_set_bits(*word);
    bitarray_print(&array.header);
    printf("Counted %zu set bits\n", setBits);

    printf("=== Counting unset bits ===\n");
    size_t unsetBits = u64_count_unset_bits(*word);
    bitarray_print(&array.header);
    printf("Counted %zu unset bits\n", unsetBits);

    printf("=== Counting leading/trailing zeros (bit 12 set) ===\n");
    *word = 0x0000000000001000ULL;
    bitarray_print(&array.header);
    size_t lz = u64_count_leading_zeros(*word);
    size_t tz = u64_count_trailing_zeros(*word);
    printf("Leading zeros: %zu, Trailing zeros: %zu\n", lz, tz);

    printf("=== Setting all 64 bits ===\n");
    *word = 0;
    u64_set_bit_range(word, 0, 64);
    bitarray_print(&array.header);

    printf("=== Clearing all 64 bits ===\n");
    u64_clear_bit_range(word, 0, 64);
    bitarray_print(&array.header);

    printf("=== Performing no-op on zero-length range ===\n");
    *word = UINT64_MAX;
    u64_clear_bit_range(word, 20, 20);
    u64_set_bit_range(word, 33, 33);

    printf("=== Setting highest bit (bit 63) ===\n");
    *word = 0;
    u64_set_bit(word, 63);
    bitarray_print(&array.header);

    printf("=== Toggling full bit range on and off ===\n");
    *word = 0;
    u64_set_bit_range(word, 0, 64);
    u64_clear_bit_range(word, 0, 64);

    printf("=== Setting bit range [1..64] and verifying no overflow ===\n");
    *word = 0;
    u64_set_bit_range(word, 1, 64);
    core_assert((*word & (1ULL << 0)) == 0);
    core_assert((*word & (1ULL << 1)) != 0);
    core_assert((*word & (1ULL << 63)) != 0);
    core_assert(u64_count_set_bits(*word) == 63);

    printf("=== All u64 util tests passed ===\n");
}
//=============================================================================

//===TESTS=====================================================================
void test_run_bitset(){
    internal_run_u64_util_tests();
}

void test_run_bitarray(){
    internal_run_bitarray_util_tests();
    internal_test_bitarray_ranges();
}
//=============================================================================
#endif
