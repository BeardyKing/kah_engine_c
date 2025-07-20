#include <stdio.h>

#include <kah_core/allocators.h>

int main(void)
{
    allocator_create();
    {
        int32_t* data = g_allocators.cstd.alloc(sizeof(int32_t));
        *data = 32;

        printf("%i\n",*data);
    }
    allocator_cleanup();
    return 0;
}
