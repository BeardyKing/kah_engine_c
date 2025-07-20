#ifndef ALLOCATOR_ARENA_H
#define ALLOCATOR_ARENA_H

//===INCLUDES==================================================================
#include <stddef.h>
//=============================================================================

//===API=======================================================================
void mem_create();
void mem_cleanup();

void* mem_cstd_alloc(size_t inBufferSize);
void mem_cstd_free(void* bufferAddress);

void* mem_arena_alloc(size_t inBufferSize);
void mem_arena_free_assert_on_call(void* bufferAddress);
void mem_arena_reset(void* bufferAddress);

void* mem_page_alloc(size_t inBufferSize);
void mem_page_free(void* bufferAddress);
//=============================================================================

#endif //ALLOCATOR_ARENA_H
