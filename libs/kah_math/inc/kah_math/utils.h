#ifndef KAH_MATH_UTILS_H
#define KAH_MATH_UTILS_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>

#include <stdint.h>
//=============================================================================

//===API=======================================================================
MATH_FORCE_INLINE int8_t   min_i8(int8_t a, int8_t b)       { return (a < b) ? a : b; }
MATH_FORCE_INLINE int16_t  min_i16(int16_t a, int16_t b)    { return (a < b) ? a : b; }
MATH_FORCE_INLINE int32_t  min_i32(int32_t a, int32_t b)    { return (a < b) ? a : b; }
MATH_FORCE_INLINE int64_t  min_i64(int64_t a, int64_t b)    { return (a < b) ? a : b; }

MATH_FORCE_INLINE uint8_t  min_u8(uint8_t a, uint8_t b)     { return (a < b) ? a : b; }
MATH_FORCE_INLINE uint16_t min_u16(uint16_t a, uint16_t b)  { return (a < b) ? a : b; }
MATH_FORCE_INLINE uint32_t min_u32(uint32_t a, uint32_t b)  { return (a < b) ? a : b; }
MATH_FORCE_INLINE uint64_t min_u64(uint64_t a, uint64_t b)  { return (a < b) ? a : b; }

MATH_FORCE_INLINE int8_t   max_i8(int8_t a, int8_t b)       { return (a > b) ? a : b; }
MATH_FORCE_INLINE int16_t  max_i16(int16_t a, int16_t b)    { return (a > b) ? a : b; }
MATH_FORCE_INLINE int32_t  max_i32(int32_t a, int32_t b)    { return (a > b) ? a : b; }
MATH_FORCE_INLINE int64_t  max_i64(int64_t a, int64_t b)    { return (a > b) ? a : b; }

MATH_FORCE_INLINE uint8_t  max_u8(uint8_t a, uint8_t b)     { return (a > b) ? a : b; }
MATH_FORCE_INLINE uint16_t max_u16(uint16_t a, uint16_t b)  { return (a > b) ? a : b; }
MATH_FORCE_INLINE uint32_t max_u32(uint32_t a, uint32_t b)  { return (a > b) ? a : b; }
MATH_FORCE_INLINE uint64_t max_u64(uint64_t a, uint64_t b)  { return (a > b) ? a : b; }

float clamp_f(float value, float min, float max);
float as_degrees_f(float radians);
float as_radians_f(float degrees);

double clamp_d(double value, double min, double max);
double as_degrees_d(double radians);
double as_radians_d(double degrees);
//=============================================================================

#endif //KAH_MATH_UTILS_H
