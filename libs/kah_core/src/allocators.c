//===INCLUDES==================================================================
#include <kah_core/allocators.h>
#include <kah_core/memory.h>
//=============================================================================

//===API=======================================================================
void allocator_create() {
    g_allocators.cstd = (Allocator){
        .alloc = mem_cstd_alloc,
        .free = mem_cstd_free,
    };

    g_allocators.arena = (Allocator){
        .alloc = mem_arena_alloc,
        .free = mem_arena_free_assert_on_call,
    };

    g_allocators.page = (Allocator){
        .alloc = mem_page_alloc,
        .free = mem_page_free,
    };
}

void allocator_cleanup() {
    g_allocators = (struct CoreAllocators){};
}
//=============================================================================
