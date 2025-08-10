//===INCLUDES==================================================================
#include <kah_core/memory.h>
#include <kah_core/assert.h>
#include <kah_core/defines.h>
#include <kah_core/utils.h>
#include <kah_core/bit_array.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
//=============================================================================

//===STRUCTS_INTERNAL==========================================================
#define ALLOC_TABLE_INVALID_INDEX UINT32_MAX

#define MEM_ARENA_BUFFER_SIZE ( 1 * KAH_MiB )
struct ArenaData
{
    char data[MEM_ARENA_BUFFER_SIZE];
    size_t current;
} typedef ArenaData;

#define MEM_MAX_DYNAMIC_ALLOCATIONS 256
struct AllocTable
{
    AllocInfo infos[MEM_MAX_DYNAMIC_ALLOCATIONS];
    BitArray_256 infoInUse;
} typedef AllocTable;
//=============================================================================

//===DATA_INTERNAL=============================================================
static AllocTable s_allocationTable = {};

static ArenaData* s_arenaData = nullptr;
static AllocInfo* s_arenaAllocInfo = nullptr;
//=============================================================================

//===INTERNAL==================================================================
void* malloc_zeroed(size_t size){
    return calloc(1, size);
}

uint32_t alloc_info_get_next_free_index(){
    const size_t bitIndex = bitarray_find_first_unset_bit(&s_allocationTable.infoInUse.header);
    core_assert_msg(bitIndex != UINT64_MAX, "err: alloc info has no free slots");
    return bitIndex != UINT64_MAX ? bitIndex : ALLOC_TABLE_INVALID_INDEX;
}

uint32_t alloc_info_find_index(const AllocInfo* allocInfo){
    for (uint32_t i = 0; i < MEM_MAX_DYNAMIC_ALLOCATIONS; ++i)
    {
        if(&s_allocationTable.infos[i] == allocInfo)
        {
            return i;
        }
    }
    return ALLOC_TABLE_INVALID_INDEX;
}

#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
static void* internal_page_alloc(size_t inBufferSize){ core_not_implemented(); return nullptr; }
void internal_page_realloc(AllocInfo* allocInfo, size_t inBufferSize){ core_not_implemented(); }
static void internal_page_free(AllocInfo* allocInfo){ core_not_implemented(); }
#endif

#if CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
static void* internal_page_alloc(size_t inBufferSize){ core_not_implemented(); return nullptr; }
void internal_page_realloc(AllocInfo* allocInfo, size_t inBufferSize){ core_not_implemented(); }
static void internal_page_free(AllocInfo* allocInfo){ core_not_implemented(); }
#endif

#if CHECK_FEATURE(FEATURE_PLATFORM_APPLE)
static void* internal_page_alloc(size_t inBufferSize){ core_not_implemented(); return nullptr; }
void internal_page_realloc(AllocInfo* allocInfo, size_t inBufferSize){ core_not_implemented(); }
static void internal_page_free(AllocInfo* allocInfo){ core_not_implemented(); }
#endif
//=============================================================================

//===API=======================================================================
AllocInfo* mem_cstd_alloc(size_t inBufferSize){
    const uint32_t tableIndex = alloc_info_get_next_free_index();
    if(tableIndex == ALLOC_TABLE_INVALID_INDEX){
        return nullptr;
    }

    void* bufferAddress = malloc_zeroed(inBufferSize);
    if( bufferAddress != nullptr )
    {
        AllocInfo* outInfo = &s_allocationTable.infos[tableIndex];
        bitarray_set_bit(&s_allocationTable.infoInUse.header, tableIndex);

        *outInfo = (AllocInfo){
            .bufferAddress = bufferAddress,
            .commitedMemory = inBufferSize,
            .reservedMemory = inBufferSize
        };
        return outInfo;
    }
    return nullptr;
}

void mem_cstd_free(AllocInfo* allocInfo){
    core_assert(allocInfo != nullptr);
    const uint32_t tableIndex = alloc_info_find_index(allocInfo);
    if(tableIndex == ALLOC_TABLE_INVALID_INDEX){
        core_assert_msg(false, "err: could not find alloc info in table");
        return;
    }
    core_assert(allocInfo->bufferAddress == s_allocationTable.infos[tableIndex].bufferAddress);
    free(allocInfo->bufferAddress);
    *allocInfo = (AllocInfo){};
    bitarray_clear_bit(&s_allocationTable.infoInUse.header, tableIndex);
}

void mem_cstd_realloc(AllocInfo* allocInfo, size_t inBufferSize){
    core_assert(allocInfo != nullptr);
    const uint32_t tableIndex = alloc_info_find_index(allocInfo);
    if(tableIndex == ALLOC_TABLE_INVALID_INDEX){
        core_assert_msg(false, "err: could not find alloc info in table");
        return;
    }

    void* bufferAddress = realloc(allocInfo->bufferAddress, inBufferSize);
    if(bufferAddress != nullptr){
        *allocInfo = (AllocInfo){
            .bufferAddress = bufferAddress,
            .commitedMemory = inBufferSize,
            .reservedMemory = inBufferSize
        };
    }
    else{
        core_assert_msg(false, "err: realloc failed");
        mem_cstd_free(allocInfo);
    }
}

AllocInfo* mem_arena_alloc(size_t inBufferSize){
    core_assert(s_arenaData != nullptr);
    core_assert(s_arenaData->current + inBufferSize < MEM_ARENA_BUFFER_SIZE);
    void* outData = &s_arenaData->data[s_arenaData->current];
    s_arenaData->current += inBufferSize;
    return outData;
}

void mem_arena_free_assert_on_call(AllocInfo* /*bufferAddres*/){
    core_assert_msg(false, "arena does not implement free, call mem_arena_reset at the end of the frame");
}

void mem_arena_realloc_assert_on_call(AllocInfo* /*allocInfo*/, size_t /*inBufferSize*/){
    core_assert_msg(false, "arena does not implement realloc, call mem_arena_reset at the end of the frame");
}

void mem_arena_reset(){
    *s_arenaData = (ArenaData){};
}

AllocInfo* mem_page_alloc(size_t inBufferSize){
    return internal_page_alloc(inBufferSize);
}

void mem_page_realloc(AllocInfo* allocInfo, size_t inBufferSize){
    internal_page_realloc(allocInfo, inBufferSize);
}

void mem_page_free(AllocInfo* allocInfo){
    internal_page_free(allocInfo);
}

bool mem_alloc_table_empty(){
    return bitarray_count_set_bits(&s_allocationTable.infoInUse.header) == 0;
}

void mem_dump_info(){
    size_t totalCommited = 0;
    size_t totalReserved = 0;
    for (uint32_t i = 0; i < MEM_MAX_DYNAMIC_ALLOCATIONS; ++i){
        const bool inUse = bitarray_check_bit(&s_allocationTable.infoInUse.header, i);
        if(inUse){
            const AllocInfo* info = &s_allocationTable.infos[i];
            printf("MEM:\t [Slot: %u] [Address: %p] [Commited: %zu] [Reserved: %zu]\n", i, info->bufferAddress, info->commitedMemory, info->reservedMemory);
            totalCommited += info->commitedMemory;
            totalReserved += info->reservedMemory;
        }
    }

    if(totalCommited > 0 || totalReserved > 0){
        printf("MEM:\t [Total Commited: %zu]\n", totalCommited);
        printf("MEM:\t [Total Reserved: %zu]\n", totalReserved);
    }
}

//=============================================================================

//===INIT/SHUTDOWN=============================================================
void mem_create(){
    {
        s_allocationTable = (AllocTable){.infoInUse.header.bitCount = 256, .infoInUse.buf = {0ULL, 0ULL, 0ULL, 0ULL}};
        s_arenaAllocInfo = mem_cstd_alloc(sizeof(ArenaData));
        s_arenaData = (ArenaData*)s_arenaAllocInfo->bufferAddress;
    }
}

void mem_cleanup(){
    {
        mem_cstd_free(s_arenaAllocInfo);
        s_arenaData = nullptr;
    }
}
//=============================================================================
