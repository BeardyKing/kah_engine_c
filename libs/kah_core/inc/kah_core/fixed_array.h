#ifndef FIXED_ARRAY_H
#define FIXED_ARRAY_H

//===INCLUDES==================================================================
#include <kah_core/allocators.h>

#include <stdint.h>
//=============================================================================

//===TYPES=====================================================================
struct FixedArray{
    AllocInfo* info;
    uint32_t count;
    uint32_t typeSize;
}typedef FixedArray;
//=============================================================================

//===API=======================================================================
void fixed_array_insert(FixedArray* fixedArray, uint32_t index, void* data);
void* fixed_array_get(FixedArray* fixedArray, uint32_t index);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
FixedArray fixed_array_create(Allocator allocator, uint32_t typeSize, uint32_t count);
void fixed_array_cleanup(Allocator allocator, FixedArray* array);
//=============================================================================

#endif //FIXED_ARRAY_H
