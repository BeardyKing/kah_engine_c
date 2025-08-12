#ifndef VEC3_H
#define VEC3_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>

#include <math.h>
#include <stdint.h>
//=============================================================================

//===PUBLIC_STRUCTS============================================================
union vec3f{
    struct{ float x, y, z; };
    struct{ float r, g, b; };
} typedef vec3f;
//=============================================================================

//===API=======================================================================
MATH_FORCE_INLINE vec3f vec3f_add(const vec3f* a, const vec3f* b){ return (vec3f){.x = a->x + b->x,.y = a->y + b->y,.z = a->z + b->z,}; }
MATH_FORCE_INLINE vec3f vec3f_sub(const vec3f* a, const vec3f* b){ return (vec3f){.x = a->x - b->x,.y = a->y - b->y,.z = a->z - b->z,}; }
MATH_FORCE_INLINE vec3f vec3f_mul(const vec3f* a, float scalar){ return (vec3f){.x = a->x * scalar,.y = a->y * scalar,.z = a->z * scalar,}; }
MATH_FORCE_INLINE vec3f vec3f_div(const vec3f* a, float scalar){ return (vec3f){.x = a->x / scalar,.y = a->y / scalar,.z = a->z / scalar,}; }

MATH_FORCE_INLINE float vec3f_mag(const vec3f* a){ return sqrtf(a->x * a->x + a->y * a->y + a->z * a->z); }
MATH_FORCE_INLINE vec3f vec3f_norm(const vec3f* a){ return vec3f_div(a, vec3f_mag(a)); }

MATH_FORCE_INLINE float vec3f_dot(const vec3f* a, const vec3f* b) { return (a->x * b->x) + (a->y * b->y) + (a->z * b->z); }
MATH_FORCE_INLINE vec3f vec3f_cross(const vec3f* a, const vec3f* b) { return (vec3f){.x = (a->y * b->z) - (a->z * b->y),.y = (a->z * b->x) - (a->x * b->z),.z = (a->x * b->y) - (a->y * b->x),}; }
//=============================================================================

#endif //VEC3_H
