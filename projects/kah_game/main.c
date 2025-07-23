#include <stdio.h>
#include <stdint.h>

#include <kah_core/allocators.h>
#include <kah_core/defines.h>
#include <kah_core/assert.h>

int main(void)
{
    allocator_create();
    {
        uint64_t u64Value = INT64_MAX; // + 1;
        int64_t i64Value = truncate_cast(int64_t, u64Value);

        printf("%lld %llu \n", i64Value, i64Value);
        printf("%lld %llu \n", type_max(i64Value), type_max(u64Value));
        printf("%lld %u \n", type_min(i64Value), type_min(u64Value));

        int32_t* data = allocators()->cstd.alloc(sizeof(int32_t));
        *data = 32;

        printf("%i\n",*data);
    }
    allocator_cleanup();
    return 0;
}
