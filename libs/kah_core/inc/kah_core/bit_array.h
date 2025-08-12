#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H
//===INCLUDES==================================================================
#include <kah_core/assert.h>
#include <kah_core/utils.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
//=============================================================================

//===TYPES=====================================================================
#define KAH_BIT_ARRAY_ALIGNMENT 64

struct BitArrayHeader{
    uint64_t bitCount;
} typedef BitArrayHeader;

struct BitArray_64{     BitArrayHeader header; uint64_t buf[1];   } typedef BitArray_64;
struct BitArray_128{    BitArrayHeader header; uint64_t buf[2];   } typedef BitArray_128;
struct BitArray_256{    BitArrayHeader header; uint64_t buf[4];   } typedef BitArray_256;
struct BitArray_512{    BitArrayHeader header; uint64_t buf[8];   } typedef BitArray_512;
struct BitArray_1024{   BitArrayHeader header; uint64_t buf[16];  } typedef BitArray_1024;
struct BitArray_2048{   BitArrayHeader header; uint64_t buf[32];  } typedef BitArray_2048;
struct BitArray_4096{   BitArrayHeader header; uint64_t buf[64];  } typedef BitArray_4096;
struct BitArray_8192{   BitArrayHeader header; uint64_t buf[128]; } typedef BitArray_8192;

struct BitArrayDynamic{ BitArrayHeader header; uint64_t* buf;     } typedef BitArrayDynamic;
//=============================================================================

//===API=======================================================================
CORE_FORCE_INLINE uint64_t* bitarray_buffer(BitArrayHeader* header) { return (uint64_t*)(header + 1); }

bool   bitarray_check_bit(BitArrayHeader* header, size_t bitIndex);
void   bitarray_set_bit(BitArrayHeader* header, size_t bitIndex);
void   bitarray_clear_bit(BitArrayHeader* header, size_t bitIndex);
void   bitarray_toggle_bit(BitArrayHeader* header, size_t bitIndex);
void   bitarray_set_bit_range(BitArrayHeader* header, size_t startBit, size_t endBit);
void   bitarray_clear_bit_range(BitArrayHeader* header, size_t startBit, size_t endBit);
size_t bitarray_count_set_bits(BitArrayHeader* header);
size_t bitarray_count_unset_bits(BitArrayHeader* header);
size_t bitarray_count_trailing_zeros(BitArrayHeader* header);
size_t bitarray_count_leading_zeros(BitArrayHeader* header);
size_t bitarray_find_first_unset_bit(BitArrayHeader* header);
void   bitarray_print(BitArrayHeader* header);
//=============================================================================

//===BITSET_UTILS==============================================================
CORE_FORCE_INLINE bool   u64_is_bit_set(uint64_t word, size_t bitIndex)                     { core_assert(bitIndex < KAH_BIT_ARRAY_ALIGNMENT); return (word & (1ULL << bitIndex)) != 0; }
CORE_FORCE_INLINE void   u64_set_bit(uint64_t* word, size_t bitIndex)                       { core_assert(word != NULL); core_assert(bitIndex < KAH_BIT_ARRAY_ALIGNMENT); *word |= (1ULL << bitIndex); }
CORE_FORCE_INLINE void   u64_clear_bit(uint64_t* word, size_t bitIndex)                     { core_assert(word != NULL); core_assert(bitIndex < KAH_BIT_ARRAY_ALIGNMENT); *word &= ~(1ULL << bitIndex); }
CORE_FORCE_INLINE void   u64_toggle_bit(uint64_t* word, size_t bitIndex)                    { core_assert(word != NULL); core_assert(bitIndex < KAH_BIT_ARRAY_ALIGNMENT); *word ^= (1ULL << bitIndex); }
CORE_FORCE_INLINE size_t u64_count_set_bits(uint64_t word)                                  { return __builtin_popcountll(word);}
CORE_FORCE_INLINE size_t u64_count_unset_bits(uint64_t word)                                { return KAH_BIT_ARRAY_ALIGNMENT - u64_count_set_bits(word);}
CORE_FORCE_INLINE size_t u64_count_trailing_zeros(uint64_t word)                            { return word ? __builtin_ctzll(word) : KAH_BIT_ARRAY_ALIGNMENT; }
CORE_FORCE_INLINE size_t u64_count_leading_zeros(uint64_t word)                             { return word ? __builtin_clzll(word) : KAH_BIT_ARRAY_ALIGNMENT; }
CORE_FORCE_INLINE void   u64_clear_bit_range(uint64_t* word, size_t start, size_t end)      { core_assert(word != NULL); core_assert(start <= end); core_assert(end <= KAH_BIT_ARRAY_ALIGNMENT); if (start == end) { return; } if (start == 0 && end == 64) { *word = 0; } else { uint64_t mask = ((1ULL << (end - start)) - 1) << start; *word &= ~mask; } }
CORE_FORCE_INLINE void   u64_set_bit_range(uint64_t* word, size_t start, size_t end)        { core_assert(word != NULL); core_assert(start <= end); core_assert(end <= KAH_BIT_ARRAY_ALIGNMENT); if (start == end) { return; } if (start == 0 && end == 64) { *word = UINT64_MAX; } else { uint64_t mask = ((1ULL << (end - start)) - 1) << start; *word |= mask; } }
//=============================================================================
#endif //BIT_ARRAY_H
