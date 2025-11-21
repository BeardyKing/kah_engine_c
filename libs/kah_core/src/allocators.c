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
        .realloc = mem_cstd_realloc,
        .free = mem_cstd_free,
    };

    s_allocators.arena = (Allocator){
        .alloc = mem_arena_alloc,
        .realloc = mem_arena_realloc,
        .free = mem_arena_free_assert_on_call,
    };

    s_allocators.page = (Allocator){
        .alloc = mem_page_alloc,
        .realloc = mem_page_realloc,
        .free = mem_page_free,
    };
}

void allocator_cleanup() {
    s_allocators = (CoreAllocators){0};
}
//=============================================================================
