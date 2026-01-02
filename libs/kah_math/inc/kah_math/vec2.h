#ifndef VEC2_H
#define VEC2_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>
#include <kah_math/math_assert.h>
//=============================================================================

//===API=======================================================================
//void  vec2f_add       (   vec2f* dest,    const vec2f* v                      )
//void  vec2f_sub       (   vec2f* dest,    const vec2f* v                      )
//void  vec2f_mul       (   vec2f* dest,    const vec2f* v                      )
//void  vec2f_div       (   vec2f* dest,    const vec2f* v                      )
//void  vec2f_div_s     (   vec2f* dest,    const float scalar                  )
//void  vec2f_mul_add   (   vec2f* dest,    const vec2f* a,     const vec2f* b  )
//void  vec2f_mul_add_s (   vec2f* dest,    const float scalar, const vec2f* v  )
//void  vec2f_norm      (   vec2f* dest                                         )
//float vec2f_dot       (   const vec2f* a, const vec2f* b                      )
//float vec2f_mag       (   const vec2f* a                                      )

//bool  vec2i_equal (   const vec2i* a, const vec2i b*  )
//=============================================================================

//===PUBLIC_STRUCTS============================================================
union vec2i{ struct{ int32_t    x, y; }; struct{ int32_t    u, v; };} typedef vec2i;
union vec2u{ struct{ uint32_t   x, y; }; struct{ uint32_t   u, v; };} typedef vec2u;
union vec2f{ struct{ float      x, y; }; struct{ float      u, v; };} typedef vec2f;
union vec2d{ struct{ double     x, y; }; struct{ double     u, v; };} typedef vec2d;
//=============================================================================

//===INLINE====================================================================
//===VEC2_FLOAT================================================================
MATH_FORCE_INLINE void vec2f_add(vec2f* dest, const vec2f* v){
    dest->x += v->x;
    dest->y += v->y;
}

MATH_FORCE_INLINE void vec2f_sub(vec2f* dest, const vec2f* v){
    dest->x -= v->x;
    dest->y -= v->y;
}

MATH_FORCE_INLINE void vec2f_mul(vec2f* dest, const vec2f* v){
    dest->x *= v->x;
    dest->y *= v->y;
}

MATH_FORCE_INLINE void vec2f_div(vec2f* dest, const vec2f* v){
    math_assert(v->x != 0.0f);
    math_assert(v->y != 0.0f);
    dest->x /= v->x;
    dest->y /= v->y;
}

MATH_FORCE_INLINE void vec2f_mul_s(vec2f* dest, const float scalar){
    dest->x *= scalar;
    dest->y *= scalar;
}

MATH_FORCE_INLINE void vec2f_div_s(vec2f* dest, const float scalar){
    math_assert(scalar != 0.0f);
    dest->x /= scalar;
    dest->y /= scalar;
}

MATH_FORCE_INLINE void vec2f_mul_add(vec2f* dest, const vec2f* a, const vec2f* b){
    dest->x += a->x * b->x;
    dest->y += a->y * b->y;
}

MATH_FORCE_INLINE void vec2f_mul_add_s(vec2f* dest, const float scalar, const vec2f* v){
    dest->x += v->x * scalar;
    dest->y += v->y * scalar;
}

MATH_FORCE_INLINE float vec2f_mag(const vec2f* a){
    return sqrtf(a->x * a->x + a->y * a->y);
}

MATH_FORCE_INLINE void vec2f_norm(vec2f* dest){
    float magnitude = vec2f_mag(dest);
    math_assert(magnitude != 0.0f);
    vec2f_div_s(dest, magnitude);
}

MATH_FORCE_INLINE float vec2f_dot(const vec2f* a, const vec2f* b){
    return (a->x * b->x) + (a->y * b->y);
}
//=============================================================================

//===VEC2_INT32================================================================
MATH_FORCE_INLINE bool vec2i_equal(const vec2i* a, const vec2i* b){
    return a->x == b->x && a->y == b->y;
}
//=============================================================================
//=============================================================================

#endif //VEC2_H
