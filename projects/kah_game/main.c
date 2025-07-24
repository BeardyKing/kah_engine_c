#include <stdio.h>
#include <stdint.h>

#include <kah_core/allocators.h>
#include <kah_core/defines.h>
#include <kah_core/assert.h>
#include <kah_core/memory.h>

int main(void)
{
    mem_create();
    allocator_create();
    {
        uint64_t u64Value = INT64_MAX; // + 1;
        int64_t i64Value = truncate_cast(int64_t, u64Value);

        printf("%lld %llu \n", i64Value, i64Value);
        printf("%lld %llu \n", type_max(i64Value), type_max(u64Value));
        printf("%lld %u \n", type_min(i64Value), type_min(u64Value));
    }
    {
        AllocInfo* alloc = allocators()->cstd.alloc(sizeof(int32_t));
        int32_t* arr = (int32_t*)alloc->bufferAddress;
        *arr = 32;
        printf("%i\n",*(uint32_t*)alloc->bufferAddress);

        allocators()->cstd.realloc(alloc, sizeof(int32_t) * 10);
        for (int32_t i = 0; i < 10; ++i){
            if(i > 0){
                arr[i] = i;
            }
            printf("%i,",arr[i]);
        }
        printf("\n");

        allocators()->cstd.free(alloc);
    }
    allocator_cleanup();
    mem_cleanup();

    mem_dump_info();
    core_assert_msg(mem_alloc_table_empty(), "err: memory leaks");
    return 0;
}
