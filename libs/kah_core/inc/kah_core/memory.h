#ifndef ALLOCATOR_ARENA_H
#define ALLOCATOR_ARENA_H

//===INCLUDES==================================================================
#include <kah_core/defines.h>
#include <kah_core/allocators.h>

#include <stddef.h>
//=============================================================================

//===API=======================================================================

AllocInfo*  mem_cstd_alloc(size_t inBufferSize);
void        mem_cstd_realloc(AllocInfo* allocInfo, size_t inBufferSize);
void        mem_cstd_free(AllocInfo* allocInfo);

AllocInfo*  mem_arena_alloc(size_t inBufferSize);
void        mem_arena_realloc_assert_on_call(AllocInfo* allocInfo, size_t inBufferSize);
void        mem_arena_free_assert_on_call(AllocInfo* allocInfo);
void        mem_arena_reset();

AllocInfo*  mem_page_alloc(size_t inBufferSize);
void        mem_page_realloc(AllocInfo* allocInfo, size_t inBufferSize);
void        mem_page_free(AllocInfo* allocInfo);

CORE_FORCE_INLINE size_t mem_word_size() { return sizeof(void*); }
size_t                   mem_page_size();

bool   mem_alloc_table_empty();
void   mem_dump_info();

//=============================================================================

//===INIT/SHUTDOWN=============================================================
void mem_create();
void mem_cleanup();
//=============================================================================

#endif //ALLOCATOR_ARENA_H
