#ifndef ALLOCATOR_ARENA_H
#define ALLOCATOR_ARENA_H

//===INCLUDES==================================================================
#include <stddef.h>
//=============================================================================

//===API=======================================================================
void* mem_cstd_alloc(size_t inBufferSize);
void mem_cstd_free(void* bufferAddress);

void* mem_arena_alloc(size_t inBufferSize);
void mem_arena_free_assert_on_call(void* bufferAddress);
void mem_arena_reset();

void* mem_page_alloc(size_t inBufferSize);
void mem_page_free(void* bufferAddress);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void mem_create();
void mem_cleanup();
//=============================================================================

#endif //ALLOCATOR_ARENA_H
