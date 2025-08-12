//===INCLUDES==================================================================
#include <kah_core/fixed_array.h>
#include <kah_core/assert.h>

#include <string.h>
//=============================================================================

//===INTERNAL==================================================================
static CORE_FORCE_INLINE void* internal_get_memory_at_index(FixedArray* fixedArray, uint32_t index) {
    if (index >= fixedArray->count) {
        core_assert(false);
        return nullptr;
    }
    return &((char*)fixedArray->info->bufferAddress)[fixedArray->typeSize * index];
}
//=============================================================================

//===API=======================================================================
void fixed_array_insert(FixedArray* fixedArray, uint32_t index, void* data){
    core_assert(data != nullptr);
    core_assert(fixedArray != nullptr);
    core_assert(fixedArray->info->bufferAddress != nullptr);
    core_assert(index < fixedArray->count);
    void* dest = internal_get_memory_at_index(fixedArray, index);
    memcpy(dest, data, fixedArray->typeSize);
}

void* fixed_array_get(FixedArray* fixedArray, uint32_t index){
    core_assert(fixedArray != nullptr);
    core_assert(fixedArray->info->bufferAddress != nullptr);
    core_assert(index < fixedArray->count);
    return internal_get_memory_at_index(fixedArray, index);
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
FixedArray fixed_array_create(Allocator allocator, uint32_t typeSize, uint32_t count){
    const size_t totalAllocSize = typeSize * count;
    AllocInfo* info = allocator.alloc(totalAllocSize);
    core_assert_msg(info->bufferAddress != nullptr, "err: failed to created fixed array\n");

    if (info->bufferAddress == nullptr){
        return (FixedArray){};
    }

    return (FixedArray){
        .info = info,
        .count = count,
        .typeSize = typeSize
    };
}

void fixed_array_cleanup(Allocator allocator, FixedArray* fixedArray){
    core_assert(fixedArray != nullptr);
    core_assert(fixedArray->info->bufferAddress != nullptr);
    allocator.free(fixedArray->info);
    *fixedArray = (FixedArray){};
}
//=============================================================================

