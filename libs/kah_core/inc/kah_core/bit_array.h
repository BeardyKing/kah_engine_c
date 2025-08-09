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
__forceinline uint64_t* bitarray_buffer(BitArrayHeader* header) { return (uint64_t*)(header + 1); }
void bitarray_print(BitArrayHeader* header);
//TODO: Add more BitArray functions i.e. mirror u64 bitset utils but for bit arrays of N words in size;
//=============================================================================

//===BITSET_UTILS==============================================================
__forceinline bool   u64_is_bit_set(uint64_t word, size_t bitIndex)                     { return (word & (1ULL << bitIndex)) != 0;}
__forceinline void   u64_set_bit(uint64_t* word, size_t bitIndex)                       { core_assert(word != NULL); *word |= (1ULL << bitIndex);}
__forceinline void   u64_clear_bit(uint64_t* word, size_t bitIndex)                     { core_assert(word != NULL); *word &= ~(1ULL << bitIndex);}
__forceinline void   u64_toggle_bit(uint64_t* word, size_t bitIndex)                    { core_assert(word != NULL); *word ^= (1ULL << bitIndex);}
__forceinline size_t u64_count_set_bits(uint64_t word)                                  { return __builtin_popcountll(word);}
__forceinline size_t u64_count_unset_bits(uint64_t word)                                { return KAH_BIT_ARRAY_ALIGNMENT - u64_count_set_bits(word);}
__forceinline size_t u64_count_trailing_zeros(uint64_t word)                            { return word ? __builtin_ctzll(word) : KAH_BIT_ARRAY_ALIGNMENT; }
__forceinline size_t u64_count_leading_zeros(uint64_t word)                             { return word ? __builtin_clzll(word) : KAH_BIT_ARRAY_ALIGNMENT; }
__forceinline void   u64_clear_bit_range(uint64_t* word, size_t start, size_t end)      { core_assert(word != NULL); uint64_t mask = ((1ULL << (end - start)) - 1) << start; *word &= ~mask; }
__forceinline void   u64_set_bit_range(uint64_t* word, size_t start, size_t end)        { core_assert(word != NULL); uint64_t mask = ((1ULL << (end - start)) - 1) << start; *word |= mask;}
//=============================================================================
#endif //BIT_ARRAY_H
