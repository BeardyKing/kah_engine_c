#include <stdio.h>
#include <stdint.h>

#include <kah_core/allocators.h>

int main(void)
{
    allocator_create();
    {
        int32_t* data = allocators()->cstd.alloc(sizeof(int32_t));
        *data = 32;

        printf("%i\n",*data);
    }
    allocator_cleanup();
    return 0;
}
