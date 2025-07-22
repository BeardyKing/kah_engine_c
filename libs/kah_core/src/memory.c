//===INCLUDES==================================================================
#include <kah_core/memory.h>
#include <kah_core/assert.h>
#include <kah_core/defines.h>

#include <stdlib.h>
#include <stdint.h>
//=============================================================================

//===STRUCTS_INTERNAL==========================================================
#define MEM_ARENA_BUFFER_SIZE (1 * MiB )
struct ArenaData
{
    char data[MEM_ARENA_BUFFER_SIZE];
    size_t current;
} typedef ArenaData;

struct AllocInfo
{
    void* buffer;
    uint32_t reserved;
    uint32_t commited;
} typedef AllocInfo;

#define MEM_MAX_DYNAMIC_ALLOCATIONS 256
struct AllocTable
{
    AllocInfo infos[MEM_MAX_DYNAMIC_ALLOCATIONS];
    bool infosInUse[MEM_MAX_DYNAMIC_ALLOCATIONS]; //TODO: Replace with bit array or table with hashmap
} typedef AllocTable;
//=============================================================================

//===DATA_INTERNAL=============================================================
static AllocTable s_allocationTable = {}; // TODO: alloc tracking for page/virtual allocations
static ArenaData* s_arenaData = nullptr;
//=============================================================================

//===INTERNAL==================================================================
#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
static void* internal_page_alloc(size_t inBufferSize){ core_not_implemented(); return nullptr; }
static void internal_page_free(void* bufferAddress){ core_not_implemented(); }
#endif

#if CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
static void* internal_page_alloc(size_t inBufferSize){ core_not_implemented(); return nullptr; }
static void internal_page_free(void* bufferAddress){ core_not_implemented(); }
#endif

#if CHECK_FEATURE(FEATURE_PLATFORM_APPLE)
static void* internal_page_alloc(size_t inBufferSize){ core_not_implemented(); return nullptr; }
static void internal_page_free(void* bufferAddress){ core_not_implemented(); }
#endif
//=============================================================================

//===API=======================================================================
void* mem_cstd_alloc(size_t inBufferSize){
    return malloc(inBufferSize);
}

void mem_cstd_free(void* bufferAddress){
    free(bufferAddress);
    bufferAddress = nullptr;
}

void* mem_arena_alloc(size_t inBufferSize){
    core_assert(s_arenaData != nullptr);
    core_assert(s_arenaData->current + inBufferSize < MEM_ARENA_BUFFER_SIZE);
    void* outData = &s_arenaData->data[s_arenaData->current];
    s_arenaData->current += inBufferSize;
    return outData;
}

void mem_arena_free_assert_on_call(void* /*bufferAddres*/){
    core_assert_msg(false, "arena does not implement free, call mem_arena_reset at the end of the frame");
}

void mem_arena_reset(){
    *s_arenaData = (ArenaData){};
}

void* mem_page_alloc(size_t inBufferSize){
    return internal_page_alloc(inBufferSize);
}

void mem_page_free(void* bufferAddress){
    internal_page_free(bufferAddress);
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void mem_create(){
    s_arenaData = mem_cstd_alloc(sizeof(ArenaData));
}

void mem_cleanup(){
    mem_cstd_free(s_arenaData);
}
//=============================================================================
