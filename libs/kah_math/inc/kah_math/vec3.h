#ifndef VEC3_H
#define VEC3_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>
#include <kah_math/math_assert.h>
//=============================================================================

//===API=======================================================================
//void  vec3f_add       (   vec3f* dest,    const vec3f* v                      )
//void  vec3f_sub       (   vec3f* dest,    const vec3f* v                      )
//void  vec3f_mul       (   vec3f* dest,    const vec3f* v                      )
//void  vec3f_div       (   vec3f* dest,    const vec3f* v                      )
//void  vec3f_div_s     (   vec3f* dest,    const float scalar                  )
//void  vec3f_mul_add   (   vec3f* dest,    const vec3f* a,     const vec3f* b  )
//void  vec3f_mul_add_s (   vec3f* dest,    const float scalar, const vec3f* v  )
//void  vec3f_norm      (   vec3f* dest                                         )
//void  vec3f_cross2    (   vec3f* dest,    const vec3f* a,     const vec3f* b  )
//float vec3f_dot       (   const vec3f* a, const vec3f* b                      )
//float vec3f_mag       (   const vec3f* a                                      )
//=============================================================================

//===PUBLIC_STRUCTS============================================================
union vec3f{
    struct{ float x, y, z; };
    struct{ float r, g, b; };
} typedef vec3f;

#define VEC3F_WORLD_UP      ((vec3f){ 0.0f, 1.0f,  0.0f })
#define VEC3F_WORLD_RIGHT   ((vec3f){ 1.0f, 0.0f,  0.0f })
#define VEC3F_WORLD_FORWARD ((vec3f){ 0.0f, 0.0f, -1.0f })
//=============================================================================

//===INLINE====================================================================
MATH_FORCE_INLINE void vec3f_add(vec3f* dest, const vec3f* v){
    dest->x += v->x;
    dest->y += v->y;
    dest->z += v->z;
}

MATH_FORCE_INLINE void vec3f_sub(vec3f* dest, const vec3f* v){
    dest->x -= v->x;
    dest->y -= v->y;
    dest->z -= v->z;
}

MATH_FORCE_INLINE void vec3f_mul(vec3f* dest, const vec3f* v){
    dest->x *= v->x;
    dest->y *= v->y;
    dest->z *= v->z;
}

MATH_FORCE_INLINE void vec3f_div(vec3f* dest, const vec3f* v){
    math_assert(v->x != 0.0f);
    math_assert(v->y != 0.0f);
    math_assert(v->z != 0.0f);
    dest->x /= v->x;
    dest->y /= v->y;
    dest->z /= v->z;
}

MATH_FORCE_INLINE void vec3f_mul_s(vec3f* dest, const float scalar){
    math_assert(scalar != 0.0f);
    dest->x *= scalar;
    dest->y *= scalar;
    dest->z *= scalar;
}

MATH_FORCE_INLINE void vec3f_div_s(vec3f* dest, const float scalar){
    dest->x /= scalar;
    dest->y /= scalar;
    dest->z /= scalar;
}

MATH_FORCE_INLINE void vec3f_mul_add(vec3f* dest, const vec3f* a, const vec3f* b){
    dest->x += a->x * b->x;
    dest->y += a->y * b->y;
    dest->z += a->z * b->z;
}

MATH_FORCE_INLINE void vec3f_mul_add_s(vec3f* dest, const float scalar, const vec3f* v){
    dest->x += v->x * scalar;
    dest->y += v->y * scalar;
    dest->z += v->z * scalar;
}

MATH_FORCE_INLINE float vec3f_mag(const vec3f* a){
    return sqrtf(a->x * a->x + a->y * a->y + a->z * a->z);
}

MATH_FORCE_INLINE void vec3f_norm(vec3f* dest){
    float magnitude = vec3f_mag(dest);
    math_assert(magnitude != 0.0f);
    vec3f_div_s(dest, magnitude);
}

MATH_FORCE_INLINE float vec3f_dot(const vec3f* a, const vec3f* b){
    return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

MATH_FORCE_INLINE void vec3f_cross2(vec3f* dest, const vec3f* a, const vec3f* b){
        dest->x = (a->y * b->z) - (a->z * b->y);
        dest->y = (a->z * b->x) - (a->x * b->z);
        dest->z = (a->x * b->y) - (a->y * b->x);
}
//=============================================================================

#endif //VEC3_H
