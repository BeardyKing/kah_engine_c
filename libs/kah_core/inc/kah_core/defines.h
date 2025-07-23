#ifndef DEFINES_H
#define DEFINES_H

//===INCLUDES==================================================================
#include <kah_core/assert.h>

#include <stdint.h>
#include <stdbool.h>
//=============================================================================

//===SIZES=====================================================================
#define KiB (1024)
#define MiB (KiB * KiB)
#define GiB (MiB * MiB)
#define TiB (GiB * GiB)
//=============================================================================

//===FEATURES==================================================================
#define FEATURE_ON ==
#define FEATURE_OFF !=
#define CHECK_FEATURE(FEATURE_STATE) (true FEATURE_STATE true)

#if defined(_WIN32) || defined(_WIN64)
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__linux__)
#define FEATURE_PLATFORM_WINDOWS FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_ON
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__APPLE__)
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#endif
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

#endif //DEFINES_H
