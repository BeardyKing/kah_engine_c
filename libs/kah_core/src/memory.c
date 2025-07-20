#include <kah_core/memory.h>
#include <kah_core/assert.h>

#include <stdlib.h>

void mem_create(){
    CORE_DEBUG_BREAK();
}

void mem_cleanup(){
    CORE_DEBUG_BREAK();
}

void* mem_cstd_alloc(size_t inBufferSize){
    return malloc(inBufferSize);
}

void mem_cstd_free(void* bufferAddress){
    free(bufferAddress);
}

void* mem_arena_alloc(size_t inBufferSize){
    return NULL;
}

void mem_arena_free_assert_on_call(void* bufferAddres){
    CORE_DEBUG_BREAK();
}

void mem_arena_reset(void* bufferAddress){
    CORE_DEBUG_BREAK();
}

void* mem_page_alloc(size_t inBufferSize){
    CORE_DEBUG_BREAK();
    return NULL;
}

void mem_page_free(void* bufferAddress){
    CORE_DEBUG_BREAK();
}
