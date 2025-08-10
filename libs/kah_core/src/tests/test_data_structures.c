#include <stdlib.h>
#include <kah_core/defines.h>
#if CHECK_FEATURE(FEATURE_RUN_TESTS)

//===INCLUDES==================================================================
#include <kah_core/tests/test_data_structures.h>
#include <kah_core/allocators.h>
#include <kah_core/memory.h>
#include <kah_core/fixed_array.h>
#include <kah_core/assert.h>

#include <stdio.h>
//=============================================================================

//====INTERNAL=================================================================
void internal_run_fixed_array_tests() {
    printf("=== Testing FixedArray create, insert, and get ===\n");

    FixedArray intArr = fixed_array_create(allocators()->cstd, sizeof(uint32_t), 10);
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

    fixed_array_cleanup(allocators()->cstd, &intArr);
    printf("FixedArray cleanup successful\n");
    printf("=== FixedArray tests passed ===\n\n");
}

void internal_run_allocator_tests() {
    printf("=== Testing cstd Allocator alloc, realloc, and free ===\n");

    AllocInfo* alloc = allocators()->cstd.alloc(sizeof(int32_t));
    int32_t* arr = (int32_t*)alloc->bufferAddress;
    *arr = 32;

    printf("Allocated 1 int32_t | Value: %d\n", arr[0]);
    core_assert(arr[0] == 32);

    allocators()->cstd.realloc(alloc, sizeof(int32_t) * 10);
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

    allocators()->cstd.free(alloc);
    printf("Allocator freed successfully\n");
    printf("=== Allocator tests passed ===\n\n");
}

#define MAX_LIVE_ALLOCS 254 //-1 for reserved arena alloc
#define TOTAL_OPS 1000

void internal_run_allocator_tests_rand() {
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
            AllocInfo* info = allocators()->cstd.alloc(sizeof(int32_t) * count);
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
            allocators()->cstd.realloc(alloc->info, sizeof(int32_t) * newCount);
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
            allocators()->cstd.free(liveAllocs[index].info);
            printf("Free [%zu]\n", index);

            liveAllocs[index] = liveAllocs[--liveCount];
            totalOps++;
        }
    }

    for (size_t i = 0; i < liveCount; ++i) {
        allocators()->cstd.free(liveAllocs[i].info);
    }

    printf("Allocator stress test complete: %zu operations, %zu live allocations cleaned up\n", totalOps, liveCount);
    printf("=== Extended allocator tests passed ===\n\n");
}
//=============================================================================

//===TESTS=====================================================================
void test_run_data_structures() {
    printf("=== Running data structure tests ===\n");
    internal_run_allocator_tests();
    internal_run_allocator_tests_rand();
    internal_run_fixed_array_tests();
    printf("=== All data structure tests completed ===\n");
}
//=============================================================================

#endif
