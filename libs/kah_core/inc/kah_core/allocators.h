#ifndef MEMORY_H
#define MEMORY_H

//===INCLUDES==================================================================
#include <stddef.h>
//=============================================================================

//===TYPES=====================================================================
typedef void* (*alloc_cb)(size_t bufferSize);
typedef void (*free_cb)(void* buffferAddress);

struct Allocator {
    alloc_cb alloc;
    free_cb free;
} typedef Allocator;
//=============================================================================

//===ALLOCATORS================================================================
struct CoreAllocators {
    Allocator cstd;
//  .alloc = mem_cstd_alloc
//  .free  = mem_cstd_free

    Allocator arena;
//  .alloc = mem_arena_alloc,
//  .free  = mem_arena_free_assert_on_call

    Allocator page;
//  .alloc = mem_page_alloc
//  .free  = mem_page_free,
} g_allocators;
//=============================================================================

//===API=======================================================================
void allocator_create();
void allocator_cleanup();
//=============================================================================

#endif //MEMORY_H
