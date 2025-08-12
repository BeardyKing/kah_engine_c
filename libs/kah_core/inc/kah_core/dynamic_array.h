#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

//===INCLUDES==================================================================
#include <kah_core/allocators.h>

#include <stdint.h>
//=============================================================================

//===TYPES=====================================================================
struct DynamicArray{
    AllocInfo* info;
    uint32_t count;
    uint32_t current;
    uint32_t typeSize;
}typedef DynamicArray;
//=============================================================================

//===API=======================================================================
void* dynamic_array_get(DynamicArray* fixedArray, uint32_t index);
void  dynamic_array_insert(DynamicArray* fixedArray, uint32_t index, void* inData);
void  dynamic_array_push(Allocator allocator, DynamicArray* fixedArray, void* inData);
void  dynamic_array_pop(DynamicArray* fixedArray, void* outData);
void  dynamic_array_pop_n(DynamicArray* fixedArray, uint32_t popCount);
void  dynamic_array_resize(Allocator allocator, DynamicArray* array , uint32_t count);
void  dynamic_array_resize_truncate(Allocator allocator, DynamicArray* array, uint32_t count);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
DynamicArray dynamic_array_create(Allocator allocator, uint32_t typeSize, uint32_t count);
void         dynamic_array_cleanup(Allocator allocator, DynamicArray* array);
//=============================================================================

#endif //DYNAMIC_ARRAY_H
