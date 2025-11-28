#ifndef KAH_MATH_UTILS_H
#define KAH_MATH_UTILS_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>
#include <kah_math/vec2.h>

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

MATH_FORCE_INLINE float    min_f32(float a, float b)        { return (a < b) ? a : b; }
MATH_FORCE_INLINE double   min_f64(double a, double b)      { return (a < b) ? a : b; }


MATH_FORCE_INLINE int8_t   max_i8(int8_t a, int8_t b)       { return (a > b) ? a : b; }
MATH_FORCE_INLINE int16_t  max_i16(int16_t a, int16_t b)    { return (a > b) ? a : b; }
MATH_FORCE_INLINE int32_t  max_i32(int32_t a, int32_t b)    { return (a > b) ? a : b; }
MATH_FORCE_INLINE int64_t  max_i64(int64_t a, int64_t b)    { return (a > b) ? a : b; }

MATH_FORCE_INLINE uint8_t  max_u8(uint8_t a, uint8_t b)     { return (a > b) ? a : b; }
MATH_FORCE_INLINE uint16_t max_u16(uint16_t a, uint16_t b)  { return (a > b) ? a : b; }
MATH_FORCE_INLINE uint32_t max_u32(uint32_t a, uint32_t b)  { return (a > b) ? a : b; }
MATH_FORCE_INLINE uint64_t max_u64(uint64_t a, uint64_t b)  { return (a > b) ? a : b; }

MATH_FORCE_INLINE float    max_f32(float a, float b)        { return (a > b) ? a : b; }
MATH_FORCE_INLINE double   max_f64(double a, double b)      { return (a > b) ? a : b; }


MATH_FORCE_INLINE int8_t  clamp_i8(int8_t value, int8_t min, int8_t max)         { return (value < min) ? min : (value > max) ? max : value; }
MATH_FORCE_INLINE int16_t clamp_i16(int16_t value, int16_t min, int16_t max)     { return (value < min) ? min : (value > max) ? max : value; }
MATH_FORCE_INLINE int32_t clamp_i32(int32_t value, int32_t min, int32_t max)     { return (value < min) ? min : (value > max) ? max : value; }
MATH_FORCE_INLINE int64_t clamp_i64(int64_t value, int64_t min, int64_t max)     { return (value < min) ? min : (value > max) ? max : value; }

MATH_FORCE_INLINE uint8_t  clamp_u8(uint8_t value, uint8_t min, uint8_t max)     { return (value < min) ? min : (value > max) ? max : value; }
MATH_FORCE_INLINE uint16_t clamp_u16(uint16_t value, uint16_t min, uint16_t max) { return (value < min) ? min : (value > max) ? max : value; }
MATH_FORCE_INLINE uint32_t clamp_u32(uint32_t value, uint32_t min, uint32_t max) { return (value < min) ? min : (value > max) ? max : value; }
MATH_FORCE_INLINE uint64_t clamp_u64(uint64_t value, uint64_t min, uint64_t max) { return (value < min) ? min : (value > max) ? max : value; }

MATH_FORCE_INLINE float   clamp_f32(float value, float min, float max)           { return (value < min) ? min : (value > max) ? max : value; }
MATH_FORCE_INLINE double  clamp_f64(double value, double min, double max)        { return (value < min) ? min : (value > max) ? max : value; }

MATH_FORCE_INLINE vec2i   clamp_vec2i(vec2i value, vec2i min, vec2i max)         { return (vec2i) { .x = clamp_i32(value.x, min.x, max.x), .y = clamp_i32(value.y, min.y, max.y) }; }

float as_degrees_f(float radians);
float as_radians_f(float degrees);

double as_degrees_d(double radians);
double as_radians_d(double degrees);
//=============================================================================

#endif //KAH_MATH_UTILS_H
