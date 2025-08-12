//===INCLUDES==================================================================
#include <kah_core/dynamic_array.h>
#include <kah_core/assert.h>
#include <kah_core/memory.h>
#include <kah_core/utils.h>

#include <stdlib.h>
#include <string.h>
//=============================================================================

//===INTERNAL==================================================================
static CORE_FORCE_INLINE void* internal_get_memory_at_index(DynamicArray* array, uint32_t index){
    if (index >= array->count){
        core_assert(false);
        return nullptr;
    }
    return &((char*)array->info->bufferAddress)[array->typeSize * index];
}
//=============================================================================

//===API=======================================================================
void dynamic_array_insert(DynamicArray* array, uint32_t index, void* inData){
    core_assert(inData != nullptr);
    core_assert(array != nullptr);
    core_assert(array->info->bufferAddress != nullptr);
    core_assert(index < array->count);
    void* dest = internal_get_memory_at_index(array, index);
    memcpy(dest, inData, array->typeSize);
}

void* dynamic_array_get(DynamicArray* array, uint32_t index){
    core_assert(array != nullptr);
    core_assert(array->info->bufferAddress != nullptr);
    core_assert(index < array->count);
    core_assert(index <= array->current);
    return internal_get_memory_at_index(array, index);
}

void dynamic_array_push(Allocator allocator, DynamicArray* array, void* inData){
    core_assert(inData != nullptr);
    core_assert(array != nullptr);
    core_assert(array->info->bufferAddress != nullptr);
    if (array->current + 1 > array->count){
        // TODO: Consider the resize scalar 1.5f be stored as a global so it can be modified.
        dynamic_array_resize(allocator, array, array->count * 2);
    }
    void* dest = internal_get_memory_at_index(array, array->current);
    memcpy(dest, inData, array->typeSize);
    array->current++;
}

void dynamic_array_pop(DynamicArray* array, void* outData){
    core_assert(array != nullptr);
    core_assert(array->current > 0);
    core_assert(array->info->bufferAddress != nullptr);
    void* lastEntry = internal_get_memory_at_index(array, array->current - 1);
    if (outData != nullptr){
        memcpy(outData, lastEntry, array->typeSize);
    }
#if KAH_DEBUG
    memset(lastEntry, 0, array->typeSize);
#endif
    array->current--;
}

void dynamic_array_pop_n(DynamicArray* array, uint32_t popCount) {
    core_assert(array != nullptr);
    core_assert(array->info->bufferAddress != nullptr);
    core_assert(popCount > 0);
    core_assert(array->current >= popCount);

#if KAH_DEBUG
    void* start = internal_get_memory_at_index(array, array->current - popCount);
    memset(start, 0, popCount * array->typeSize);
#endif

    array->current -= popCount;
}

void dynamic_array_resize_truncate(Allocator allocator, DynamicArray* array, uint32_t count){
    core_assert(array != nullptr);
    core_assert(array->info->bufferAddress != nullptr);
    core_assert(count > 0);

    if (array->count == count){
        return;
    }

    const size_t totalAllocSize = align_up(array->typeSize * count, mem_word_size());
    allocator.realloc(array->info, totalAllocSize);
    core_assert_msg(array->info->bufferAddress != nullptr, "err: failed to resize dynamic array\n");
    array->count = count;
    if(array->current > count){
        array->current = count;
    }

    if (array->info->bufferAddress == nullptr){
        *array = (DynamicArray){};
    }
}

void dynamic_array_resize(Allocator allocator, DynamicArray* array, uint32_t count){
    core_assert(array != nullptr);
    core_assert(array->info->bufferAddress != nullptr);
    core_assert_msg(array->current < count, "err: resize down still has data in array");
    core_assert(count > 0);
    dynamic_array_resize_truncate(allocator, array, count);
}

//=============================================================================

//===INIT/SHUTDOWN=============================================================

DynamicArray dynamic_array_create(Allocator allocator, uint32_t typeSize, uint32_t count){
    const size_t totalAllocSize = typeSize * count;
    AllocInfo* info = allocator.alloc(totalAllocSize);
    core_assert_msg(info->bufferAddress != nullptr, "err: failed to created dynamic array\n");

    if (info->bufferAddress == nullptr){
        return (DynamicArray){};
    }

    return (DynamicArray){
        .info = info,
        .count = count,
        .current = 0,
        .typeSize = typeSize
    };
}

void dynamic_array_cleanup(Allocator allocator, DynamicArray* array){
    core_assert(array != nullptr);
    core_assert(array->info->bufferAddress != nullptr);
    allocator.free(array->info);
    *array = (DynamicArray){};
}
//=============================================================================

