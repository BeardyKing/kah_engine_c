//===INCLUDES==================================================================
#include <kah_core/allocators.h>
#include <kah_core/memory.h>
//=============================================================================

//===DATA_INTERNAL=============================================================
static CoreAllocators s_allocators;
//=============================================================================

//===API=======================================================================
CoreAllocators* allocators()
{
    return &s_allocators;
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void allocator_create() {
    s_allocators.cstd = (Allocator){
        .alloc = mem_cstd_alloc,
        .free = mem_cstd_free,
    };

    s_allocators.arena = (Allocator){
        .alloc = mem_arena_alloc,
        .free = mem_arena_free_assert_on_call,
    };

    s_allocators.page = (Allocator){
        .alloc = mem_page_alloc,
        .free = mem_page_free,
    };
}

void allocator_cleanup() {
    s_allocators = (CoreAllocators){};
}
//=============================================================================
