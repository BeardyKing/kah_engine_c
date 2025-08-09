//===INCLUDES==================================================================
#include <kah_core/bit_array.h>
#include <kah_core/utils.h>
#include <kah_core/assert.h>

#include <stdio.h>
#include <stdint.h>
//=============================================================================

//===API=======================================================================
void bitarray_print(BitArrayHeader* header) {
    core_assert(header != nullptr);
    uint64_t* buf = bitarray_buffer(header);
    size_t bitCount = header->bitCount;
    size_t wordCount = align_up(header->bitCount,KAH_BIT_ARRAY_ALIGNMENT) / KAH_BIT_ARRAY_ALIGNMENT;

    printf("BitArray (%zu bits) : (%zu words)\n", bitCount, wordCount);

    for (size_t wordIndex = 0; wordIndex < wordCount; ++wordIndex) {
        char bitStr[KAH_BIT_ARRAY_ALIGNMENT + (KAH_BIT_ARRAY_ALIGNMENT / 4) + 1]; // bitCount + spacing + '/0'
        size_t bitStrIndex = 0;

        for (int32_t bit = (KAH_BIT_ARRAY_ALIGNMENT - 1 ); bit >= 0; --bit) {
            size_t bitIndex = (wordIndex * KAH_BIT_ARRAY_ALIGNMENT) + bit;
            if (bitIndex >= bitCount) {
                bitStr[bitStrIndex++] = '?';
            } else {
                bitStr[bitStrIndex++] = (buf[wordIndex] & (1ULL << bit)) ? '1' : '0';
            }

            if (bit % 4 == 0 && bit != 0) {
                bitStr[bitStrIndex++] = ' ';
            }
        }

        bitStr[bitStrIndex] = '\0';
        printf("word[%zd]: %s\n", wordIndex, bitStr);
    }
}
//=============================================================================