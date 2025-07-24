#ifndef MEMORY_H
#define MEMORY_H

//===INCLUDES==================================================================
#include <stddef.h>
//=============================================================================

//===TYPES=====================================================================
struct AllocInfo
{
    void* bufferAddress;
    size_t commitedMemory;
    size_t reservedMemory;
} typedef AllocInfo;

typedef AllocInfo* (*alloc_cb)(size_t inBufferSize);
typedef void (*realloc_cb)(AllocInfo* allocInfo, size_t inBufferSize);
typedef void (*free_cb)(AllocInfo* allocInfo);

struct Allocator {
    alloc_cb alloc;
    realloc_cb realloc;
    free_cb free;
} typedef Allocator;
//=============================================================================

//===ALLOCATORS================================================================
struct CoreAllocators {
    Allocator cstd;
//  .alloc = mem_cstd_alloc,
//  .realloc = mem_cstd_realloc,
//  .free = mem_cstd_free,

    Allocator arena;
//  .alloc = mem_arena_alloc,
//  .realloc = mem_arena_realloc_assert_on_call,
//  .free = mem_arena_free_assert_on_call,

    Allocator page;
//  .alloc = mem_page_alloc,
//  .realloc = mem_page_realloc,
//  .free = mem_page_free,
} typedef CoreAllocators;
//=============================================================================

//===API=======================================================================
CoreAllocators* allocators();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void allocator_create();
void allocator_cleanup();
//=============================================================================

#endif //MEMORY_H
