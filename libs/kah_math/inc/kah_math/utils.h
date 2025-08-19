#ifndef KAH_MATH_UTILS_H
#define KAH_MATH_UTILS_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>

#include <stdint.h>
//=============================================================================

//===API=======================================================================
MATH_FORCE_INLINE int32_t min_i32(int32_t a, int32_t b) { return (a < b) ? a : b; }
MATH_FORCE_INLINE int32_t max_i32(int32_t a, int32_t b) { return (a > b) ? a : b; }

float clamp_f(float value, float min, float max);
float as_degrees_f(float radians);
float as_radians_f(float degrees);

double clamp_d(double value, double min, double max);
double as_degrees_d(double radians);
double as_radians_d(double degrees);
//=============================================================================

#endif //KAH_MATH_UTILS_H
