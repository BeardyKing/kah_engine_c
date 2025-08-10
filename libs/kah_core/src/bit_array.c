//===INCLUDES==================================================================
#include <kah_core/bit_array.h>
#include <kah_core/utils.h>
#include <kah_core/assert.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
//=============================================================================

//===INTERNAL==================================================================
__forceinline void internal_set_last_word_bits(uint64_t* word, size_t startOffset, size_t endOffset) {
    if (endOffset == 0) {
        *word = UINT64_MAX;
    } else {
        u64_set_bit_range(word, startOffset, endOffset);
    }
}

__forceinline void internal_clear_last_word_bits(uint64_t* word, size_t startOffset, size_t endOffset) {
    if (endOffset == 0) {
        *word = 0;
    } else {
        u64_clear_bit_range(word, startOffset, endOffset);
    }
}
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


bool bitarray_check_bit(BitArrayHeader* header, size_t bitIndex) {
    core_assert(header != nullptr);
    core_assert(bitIndex < header->bitCount);

    uint64_t* buf = bitarray_buffer(header);
    size_t wordIndex = bitIndex / KAH_BIT_ARRAY_ALIGNMENT;
    size_t bitOffset = bitIndex % KAH_BIT_ARRAY_ALIGNMENT;

    return u64_is_bit_set(buf[wordIndex], bitOffset);
}

void bitarray_set_bit(BitArrayHeader* header, size_t bitIndex) {
    core_assert(header != nullptr);
    core_assert(bitIndex < header->bitCount);

    uint64_t* buf = bitarray_buffer(header);
    size_t wordIndex = bitIndex / KAH_BIT_ARRAY_ALIGNMENT;
    size_t bitOffset = bitIndex % KAH_BIT_ARRAY_ALIGNMENT;

    u64_set_bit(&buf[wordIndex], bitOffset);
}

void bitarray_clear_bit(BitArrayHeader* header, size_t bitIndex) {
    core_assert(header != nullptr);
    core_assert(bitIndex < header->bitCount);

    uint64_t* buf = bitarray_buffer(header);
    size_t wordIndex = bitIndex / KAH_BIT_ARRAY_ALIGNMENT;
    size_t bitOffset = bitIndex % KAH_BIT_ARRAY_ALIGNMENT;

    u64_clear_bit(&buf[wordIndex], bitOffset);
}

void bitarray_toggle_bit(BitArrayHeader* header, size_t bitIndex) {
    core_assert(header != nullptr);
    core_assert(bitIndex < header->bitCount);

    uint64_t* buf = bitarray_buffer(header);
    size_t wordIndex = bitIndex / KAH_BIT_ARRAY_ALIGNMENT;
    size_t bitOffset = bitIndex % KAH_BIT_ARRAY_ALIGNMENT;

    u64_toggle_bit(&buf[wordIndex], bitOffset);
}

size_t bitarray_count_set_bits(BitArrayHeader* header) {
    core_assert(header != nullptr);

    uint64_t* buf = bitarray_buffer(header);
    size_t totalBits = header->bitCount;
    size_t wordCount = align_up(header->bitCount,KAH_BIT_ARRAY_ALIGNMENT) / KAH_BIT_ARRAY_ALIGNMENT;
    size_t count = 0;

    for (size_t i = 0; i < wordCount; ++i) {
        uint64_t word = buf[i];

        if (i == wordCount - 1) {
            size_t bitsInLastWord = totalBits % KAH_BIT_ARRAY_ALIGNMENT;
            if (bitsInLastWord != 0) {
                uint64_t mask = (1ULL << bitsInLastWord) - 1;
                word &= mask;
            }
        }

        count += u64_count_set_bits(word);
    }

    return count;
}

void bitarray_set_bit_range(BitArrayHeader* header, size_t startBit, size_t endBit) {
    core_assert(header != nullptr);
    core_assert(endBit <= header->bitCount);
    core_assert(startBit <= endBit);

    if (startBit == endBit){
        return;
    }

    uint64_t* buf = bitarray_buffer(header);

    size_t startWord = startBit / KAH_BIT_ARRAY_ALIGNMENT;
    size_t endWord = (endBit - 1) / KAH_BIT_ARRAY_ALIGNMENT;

    size_t startOffset = startBit % KAH_BIT_ARRAY_ALIGNMENT;
    size_t endOffset = endBit % KAH_BIT_ARRAY_ALIGNMENT;

    if (startWord == endWord) {
        internal_set_last_word_bits(&buf[endWord], startOffset, endOffset);
    }
    else {
        u64_set_bit_range(&buf[startWord], startOffset, 64);

        size_t fullWordStart = startWord + 1;
        size_t fullWordEnd   = endWord;

        if (fullWordStart < fullWordEnd) {
            size_t fullWordCount = fullWordEnd - fullWordStart;
            memset(&buf[fullWordStart], 0xFF, fullWordCount * sizeof(uint64_t));
        }

        internal_set_last_word_bits(&buf[endWord], 0, endOffset);
    }
}

void bitarray_clear_bit_range(BitArrayHeader* header, size_t startBit, size_t endBit) {
    core_assert(header != nullptr);
    core_assert(endBit <= header->bitCount);
    core_assert(startBit <= endBit);

    if (startBit == endBit){
        return;
    }

    uint64_t* buf = bitarray_buffer(header);

    size_t startWord = startBit / KAH_BIT_ARRAY_ALIGNMENT;
    size_t endWord = (endBit - 1) / KAH_BIT_ARRAY_ALIGNMENT;

    size_t startOffset = startBit % KAH_BIT_ARRAY_ALIGNMENT;
    size_t endOffset = endBit % KAH_BIT_ARRAY_ALIGNMENT;

    if (startWord == endWord) {
        internal_clear_last_word_bits(&buf[startWord], startOffset, endOffset);
    }
    else {
        u64_clear_bit_range(&buf[startWord], startOffset, KAH_BIT_ARRAY_ALIGNMENT);

        size_t fullWordStart = startWord + 1;
        size_t fullWordEnd = endWord;
        if (fullWordStart < fullWordEnd) {
            size_t fullWordCount = fullWordEnd - fullWordStart;
            memset(&buf[fullWordStart], 0, fullWordCount * sizeof(uint64_t));
        }

        internal_clear_last_word_bits(&buf[endWord], 0, endOffset);
    }
}

size_t bitarray_count_unset_bits(BitArrayHeader* header) {
    core_assert(header != nullptr);

    size_t totalBits = header->bitCount;
    return totalBits - bitarray_count_set_bits(header);
}

size_t bitarray_count_trailing_zeros(BitArrayHeader* header) {
    core_assert(header != nullptr);

    uint64_t* buf = bitarray_buffer(header);
    size_t wordCount = align_up(header->bitCount,KAH_BIT_ARRAY_ALIGNMENT) / KAH_BIT_ARRAY_ALIGNMENT;

    for (size_t i = 0; i < wordCount; ++i) {
        if (buf[i] != 0) {
            return (i * KAH_BIT_ARRAY_ALIGNMENT) + u64_count_trailing_zeros(buf[i]);
        }
    }

    return header->bitCount;
}

size_t bitarray_count_leading_zeros(BitArrayHeader* header) {
    core_assert(header != nullptr);

    uint64_t* buf = bitarray_buffer(header);
    size_t totalBits = header->bitCount;
    size_t wordCount = align_up(header->bitCount,KAH_BIT_ARRAY_ALIGNMENT) / KAH_BIT_ARRAY_ALIGNMENT;

    size_t lastWordBits = totalBits % KAH_BIT_ARRAY_ALIGNMENT;
    if (lastWordBits == 0){
        lastWordBits = KAH_BIT_ARRAY_ALIGNMENT;
    }

    for (size_t i = wordCount; i-- > 0;) {
        if (buf[i] != 0) {
            if (i == wordCount - 1) {
                // mask end bits in word if count doesn't perfectly match word bit count
                uint64_t mask = (lastWordBits == KAH_BIT_ARRAY_ALIGNMENT) ? UINT64_MAX : ((1ULL << lastWordBits) - 1);
                return (wordCount - 1 - i) * KAH_BIT_ARRAY_ALIGNMENT + u64_count_leading_zeros(buf[i] & mask) - (KAH_BIT_ARRAY_ALIGNMENT - lastWordBits);
            }
            else
                {
                return (wordCount - 1 - i) * KAH_BIT_ARRAY_ALIGNMENT + u64_count_leading_zeros(buf[i]);
            }
        }
    }
    return totalBits;
}

size_t bitarray_find_first_unset_bit(BitArrayHeader* header) {
    core_assert(header != nullptr);

    uint64_t* buf = bitarray_buffer(header);
    size_t totalBits = header->bitCount;
    size_t wordCount = align_up(totalBits, KAH_BIT_ARRAY_ALIGNMENT) / KAH_BIT_ARRAY_ALIGNMENT;

    for (size_t i = 0; i < wordCount; ++i) {
        if (buf[i] != UINT64_MAX) {
            uint64_t inverted = ~buf[i];
            size_t bitOffset = u64_count_trailing_zeros(inverted);
            size_t bitIndex = (i * KAH_BIT_ARRAY_ALIGNMENT) + bitOffset;

            if (bitIndex < totalBits) {
                return bitIndex;
            }
        }
    }
    core_assert_msg(false, "no free bits");
    return UINT64_MAX;
}


//=============================================================================