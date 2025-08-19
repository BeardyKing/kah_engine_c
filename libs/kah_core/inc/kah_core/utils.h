#ifndef KAH_CORE_UTILS_H
#define KAH_CORE_UTILS_H

//===INCLUDES==================================================================
#include <kah_core/assert.h>

#include <stdint.h>
#include <stdbool.h>
//=============================================================================

//===API=======================================================================
CORE_FORCE_INLINE size_t align_up(size_t size, size_t alignment)    { core_assert(alignment != 0); size_t mask = alignment - 1; return ((alignment & mask) == 0) ? ((size + mask) & ~mask) : (((size + mask)/alignment)*alignment); }
CORE_FORCE_INLINE size_t align_down(size_t size, size_t alignment)  { core_assert(alignment != 0); size_t mask = alignment - 1; return ((alignment & mask) == 0) ? (size & ~mask) : ((size / alignment) * alignment); }
CORE_FORCE_INLINE bool   aligned_to(size_t size, size_t alignment ) { core_assert(alignment != 0); return size % alignment == 0; }
CORE_FORCE_INLINE bool   is_power_of_two(size_t value)              { return ((value & (value - 1)) == 0); }//0 considered power of 2
CORE_FORCE_INLINE bool   aligned_to_ptr(void* p, size_t alignment ) { core_assert(alignment != 0); return (((uintptr_t)p % alignment) == 0); }

//val type_max( type );
//val type_min( type );
//val truncate_cast(targetType, inValue);
//=============================================================================

//===SIZES=====================================================================
#define KAH_KiB (1024)
#define KAH_MiB (KAH_KiB * KAH_KiB)
#define KAH_GiB (KAH_MiB * KAH_MiB)
#define KAH_TiB (KAH_GiB * KAH_GiB)
//=============================================================================

//===TYPES=====================================================================
#define type_max(T) _Generic((T),   \
    int8_t   : INT8_MAX,            \
    int16_t  : INT16_MAX,           \
    int32_t  : INT32_MAX,           \
    int64_t  : INT64_MAX,           \
                                    \
    uint8_t  : UINT8_MAX,           \
    uint16_t : UINT16_MAX,          \
    uint32_t : UINT32_MAX,          \
    uint64_t : UINT64_MAX           \
)

#define type_min(T) _Generic((T),   \
    int8_t   : INT8_MIN,            \
    int16_t  : INT16_MIN,           \
    int32_t  : INT32_MIN,           \
    int64_t  : INT64_MIN,           \
                                    \
    uint8_t  : 0,                   \
    uint16_t : 0,                   \
    uint32_t : 0,                   \
    uint64_t : 0                    \
)

//TODO: __int128 doesn't exist for MSVC consider a cross platform impl
#if KAH_DEBUG
    #define _truncate_cast_internal(TargetType, inValue)                                    \
    const __typeof__(inValue) value = (inValue);                                            \
    const int64_t typeMin = (int64_t)type_min((TargetType)0);                               \
    const int64_t typeMax = (int64_t)type_max((TargetType)0);                               \
    const __int128 value128 = (__int128)value;                                              \
    const bool aboveMin = value128 >= typeMin;                                              \
    const bool belowMax = value128 <= typeMax;                                              \
    const bool isSafeToTruncate = aboveMin && belowMax;                                     \
    core_assert_msg(isSafeToTruncate,                                                       \
    "truncate_cast failed: value (%lld) does not fit into '%s' [min %lld] [max %lld]",      \
    value128, #TargetType, typeMin, typeMax);
#else
#define _truncate_cast_internal(TargetType, inValue)
#endif

#define truncate_cast(TargetType, inValue)                                                  \
({                                                                                          \
    _truncate_cast_internal(TargetType, inValue);                                           \
    (TargetType)(inValue);                                                                  \
})
//=============================================================================


#endif //KAH_CORE_UTILS_H
