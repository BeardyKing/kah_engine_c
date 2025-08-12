#include <kah_core/defines.h>

#include "utils.h"

#if CHECK_FEATURE(FEATURE_RUN_TESTS)

//===INCLUDES==================================================================
#include <kah_core/tests/test_data_structures.h>
#include <kah_core/allocators.h>
#include <kah_core/memory.h>
#include <kah_core/fixed_array.h>
#include <kah_core/assert.h>
#include <kah_core/dynamic_array.h>

#include <stdio.h>
#include <stdlib.h>
//=============================================================================

//====INTERNAL=================================================================
void internal_run_fixed_array_tests(Allocator allocator) {
    printf("=== Testing FixedArray create, insert, and get ===\n");

    FixedArray intArr = fixed_array_create(allocator, sizeof(uint32_t), 10);
    void* buf = fixed_array_get(&intArr, 0);

    printf("Created FixedArray of 10 uint32_t elements\n");
    printf("Buffer (index 0): %p | Info buffer: %p | Initial value: %u\n", buf, intArr.info->bufferAddress, *(uint32_t*)buf);

    for (uint32_t i = 0; i < 10; ++i) {
        fixed_array_insert(&intArr, i, &i);
    }

    printf("Inserted values 0..9\n");
    for (uint32_t i = 0; i < 10; ++i) {
        uint32_t* data = fixed_array_get(&intArr, i);
        core_assert(data == &((uint32_t*)buf)[i]);
        core_assert(*data == i);
        printf("Index %u: %u\n", i, *data);
    }

    fixed_array_cleanup(allocator, &intArr);
    printf("FixedArray cleanup successful\n");
    printf("=== FixedArray tests passed ===\n\n");
}

void internal_run_allocator_tests(Allocator allocator) {
    printf("=== Testing cstd Allocator alloc, realloc, and free ===\n");

    AllocInfo* alloc = allocator.alloc(align_up(sizeof(int32_t), mem_word_size()));
    int32_t* arr = (int32_t*)alloc->bufferAddress;
    *arr = 32;

    printf("Allocated 1 int32_t | Value: %d\n", arr[0]);
    core_assert(arr[0] == 32);

    allocator.realloc(alloc, align_up(sizeof(int32_t) * 10,mem_word_size()));
    arr = (int32_t*)alloc->bufferAddress;

    printf("Reallocated to 10 int32_t\n");
    for (int32_t i = 0; i < 10; ++i) {
        if (i > 0) arr[i] = i;
        printf("arr[%d] = %d\n", i, arr[i]);
    }

    core_assert(arr[0] == 32);
    for (int32_t i = 1; i < 10; ++i) {
        core_assert(arr[i] == i);
    }

    allocator.free(alloc);
    printf("Allocator freed successfully\n");
    printf("=== Allocator tests passed ===\n\n");
}

#define MAX_LIVE_ALLOCS 254 //-1 for reserved arena alloc
#define TOTAL_OPS 1000

void internal_run_allocator_tests_rand(Allocator allocator) {
    printf("=== Testing cstd Allocator alloc, realloc, and free (extended) ===\n");

    typedef struct {
        AllocInfo* info;
        int32_t* ptr;
        size_t size;
    } LiveAlloc;

    LiveAlloc liveAllocs[MAX_LIVE_ALLOCS] = {0};
    size_t liveCount = 0;
    size_t totalOps = 0;

    while (totalOps < TOTAL_OPS) {
        bool canAlloc = (liveCount < MAX_LIVE_ALLOCS);
        bool canFree  = (liveCount > 0);

        int op = rand() % 3;

        if (op == 0 && canAlloc) {
            size_t count = 1 + rand() % 32;
            AllocInfo* info = allocator.alloc(align_up(sizeof(int32_t) * count,mem_word_size()));
            core_assert(info != nullptr);

            int32_t* ptr = (int32_t*)info->bufferAddress;
            for (size_t i = 0; i < count; ++i) {
                ptr[i] = (int32_t)i;
            }

            liveAllocs[liveCount++] = (LiveAlloc){ .info = info, .ptr = ptr, .size = count };
            printf("Alloc [%zu]: %zu int32_t\n", liveCount - 1, count);
            totalOps++;
        }
        else if (op == 1 && canFree) {
            size_t index = rand() % liveCount;
            LiveAlloc* alloc = &liveAllocs[index];

            size_t newCount = 1 + rand() % 64;
            allocator.realloc(alloc->info, align_up(sizeof(int32_t) * newCount,mem_word_size()));
            alloc->ptr = (int32_t*)alloc->info->bufferAddress;

            size_t minCount = (alloc->size < newCount) ? alloc->size : newCount;
            for (size_t i = 0; i < minCount; ++i) {
                core_assert(alloc->ptr[i] == (int32_t)i); // validated realloc data matches
            }

            for (size_t i = alloc->size; i < newCount; ++i) {
                alloc->ptr[i] = (int32_t)i;
            }

            alloc->size = newCount;
            printf("Realloc [%zu]: now %zu int32_t\n", index, newCount);
            totalOps++;
        }
        else if (op == 2 && canFree) {
            size_t index = rand() % liveCount;
            allocator.free(liveAllocs[index].info);
            printf("Free [%zu]\n", index);

            liveAllocs[index] = liveAllocs[--liveCount];
            totalOps++;
        }
    }

    for (size_t i = 0; i < liveCount; ++i) {
        allocator.free(liveAllocs[i].info);
    }

    printf("Allocator stress test complete: %zu operations, %zu live allocations cleaned up\n", totalOps, liveCount);
    printf("=== Extended allocator tests passed ===\n\n");
}

void internal_run_dynamic_array_tests(Allocator allocator) {
    printf("=== Testing DynamicArray ===\n");

    DynamicArray dynArr = dynamic_array_create(allocator, sizeof(uint32_t), 4);

    printf("Created DynamicArray with capacity: %u\n", dynArr.count);

    for (uint32_t i = 0; i < 4; ++i) {
        dynamic_array_insert(&dynArr, i, &i);
        // we allow writing to reserved memory,
        // but insert does not bump dynArr.current to the in index value,
    }
    dynArr.current = 4;

    for (uint32_t i = 0; i < 4; ++i) {
        uint32_t* val = (uint32_t*)dynamic_array_get(&dynArr, i);
        printf("Inserted at index %u: %u\n", i, *val);
        core_assert(*val == i);
    }

    for (uint32_t i = 4; i < 8; ++i) {
        dynamic_array_push(allocator, &dynArr, &i);
        printf("Pushed: %u\n", i);
    }

    core_assert(dynArr.count >= 8);
    core_assert(dynArr.current == 8);

    for (uint32_t i = 0; i < 8; ++i) {
        uint32_t* val = (uint32_t*)dynamic_array_get(&dynArr, i);
        printf("Verified index %u: %u\n", i, *val);
        core_assert(*val == i);
    }

    uint32_t popped = 0;
    dynamic_array_pop(&dynArr, &popped);
    printf("Popped value: %u\n", popped);
    core_assert(popped == 7);
    core_assert(dynArr.current == 7);

    dynamic_array_resize_truncate(allocator, &dynArr, 5);
    printf("Resized to 5 capacity\n");
    core_assert(dynArr.count == 5);
    core_assert(dynArr.current <= 5);

    dynamic_array_resize(allocator, &dynArr, 10);
    printf("Resized to 10 capacity\n");
    core_assert(dynArr.count == 10);
    core_assert(dynArr.current <= 5);

    dynamic_array_cleanup(allocator, &dynArr);
    printf("DynamicArray cleanup successful\n");
    printf("=== DynamicArray tests passed ===\n\n");
}
void internal_run_page_decommit_test() {
#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
    printf("=== Testing Page Allocator Page Decommit ===\n");

    const size_t pageSize = mem_page_size();
    const size_t initialPages = 4;
    const size_t shrinkToPages = 2;

    size_t initialSize = initialPages * pageSize;
    size_t shrinkSize  = shrinkToPages * pageSize;

    AllocInfo* info = mem_page_alloc(initialSize);
    core_assert(info != nullptr);
    core_assert(info->commitedMemory == initialSize);

    // Write to each page
    uint8_t* buffer = (uint8_t*)info->bufferAddress;
    for (size_t i = 0; i < initialSize; i += pageSize) {
        buffer[i] = (uint8_t)(i / pageSize); // Mark with page number
    }

    printf("Initial memory committed: %zu bytes\n", info->commitedMemory);

    // Shrink (should decommit last two pages)
    mem_page_realloc(info, shrinkSize);
    printf("Memory after shrinking: %zu bytes\n", info->commitedMemory);
    core_assert(info->commitedMemory == shrinkSize);

    // Ensure remaining memory is accessible
    for (size_t i = 0; i < shrinkSize; i += pageSize) {
        core_assert(buffer[i] == (uint8_t)(i / pageSize));
        printf("Verified buffer[%zu] = %u\n", i, buffer[i]);
    }

    // Optional: Ensure we don’t touch decommitted memory (would crash)
    // printf("Attempting to access decommitted memory (should crash)...\n");
    // buffer[shrinkSize] = 42; // Access beyond committed memory – DO NOT ENABLE unless intentionally crashing

    mem_page_free(info);
    printf("Page decommit test complete and memory freed.\n");
    printf("=== Page Decommit Test Passed ===\n\n");
#else
    printf("Skipping Page Decommit test: Not on Windows platform\n");
#endif
}

//=============================================================================

//===TESTS=====================================================================
void test_run_data_structures() {
    printf("=== Running data structure tests c standard allocator ===\n");
    {
        internal_run_dynamic_array_tests(allocators()->cstd);
        internal_run_fixed_array_tests(allocators()->cstd);

        internal_run_allocator_tests(allocators()->cstd);
        internal_run_allocator_tests_rand(allocators()->cstd);
    }
    printf("=== All data structure tests completed for c standard allocator ===\n");

    printf("=== Running data structure tests page allocator ===\n");
    {
        internal_run_dynamic_array_tests(allocators()->page);
        internal_run_fixed_array_tests(allocators()->page);

        internal_run_allocator_tests(allocators()->page);
        internal_run_allocator_tests_rand(allocators()->page);
    }
    printf("=== All data structure tests completed for page allocator ===\n");
}
//=============================================================================

#endif
