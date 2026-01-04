#ifndef VEC4_H
#define VEC4_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>
#include <kah_math/math_assert.h>
//=============================================================================

//===API=======================================================================
//uint32_t  vec4f_pack_to_uint32_t      (   const vec4f* vec            )
//vec4f     vec4f_unpack_from_uint32_t  (   const uint32_t packedValue  )

//void      vec4f_add       (   vec4f* dest,    const vec4f* v                          )
//void      vec4f_sub       (   vec4f* dest,    const vec4f* v                          )
//void      vec4f_mul       (   vec4f* dest,    const vec4f* v                          )
//void      vec4f_div       (   vec4f* dest,    const vec4f* v                          )
//void      vec4f_mul_s     (   vec4f* dest,    const float scalar                      )
//void      vec4f_div_s     (   vec4f* dest,    const float scalar                      )
//void      vec4f_mul_add   (   vec4f* dest,    const vec4f* a,     const vec4f* b      )
//void      vec4f_mul_add_s (   vec4f* dest,    const float scalar, const vec4f* v      )
//void      vec4f_norm      (   vec4f* dest                                             )
//void      vec4f_scale     (   vec4f* dest,    const vec4f* v,     const float scalar  )
//float     vec4f_dot       (   const vec4f* a, const vec4f* b                          )
//float     vec4f_mag       (   const vec4f* v                                          )
//=============================================================================

//===PUBLIC_STRUCTS============================================================
union vec4f{
    struct{ float x, y, z, w; };
    struct{ float r, g, b, a; };
} typedef vec4f;
//=============================================================================

//===INLINE====================================================================
MATH_FORCE_INLINE uint32_t vec4f_pack_to_uint32_t(const vec4f* vec){
    uint32_t packedValue = {0};
    packedValue |= (uint32_t)((uint8_t)(vec->x * 255.0f)) << 24;
    packedValue |= (uint32_t)((uint8_t)(vec->y * 255.0f)) << 16;
    packedValue |= (uint32_t)((uint8_t)(vec->z * 255.0f)) << 8;
    packedValue |= (uint32_t)((uint8_t)(vec->w * 255.0f)) << 0;
    return packedValue;
}

MATH_FORCE_INLINE vec4f vec4f_unpack_from_uint32_t(const uint32_t packedValue){
    return (vec4f){
        .x = (float)(packedValue >> 24 & 0xFF) / 255.0f,
        .y = (float)(packedValue >> 16 & 0xFF) / 255.0f,
        .z = (float)(packedValue >> 8 & 0xFF) / 255.0f,
        .w = (float)(packedValue >> 0 & 0xFF) / 255.0f,
    };
}

MATH_FORCE_INLINE void vec4f_add(vec4f* dest, const vec4f* v){
    dest->x += v->x;
    dest->y += v->y;
    dest->z += v->z;
    dest->w += v->w;
}

MATH_FORCE_INLINE void vec4f_sub(vec4f* dest, const vec4f* v){
    dest->x -= v->x;
    dest->y -= v->y;
    dest->z -= v->z;
    dest->w -= v->w;
}

MATH_FORCE_INLINE void vec4f_mul(vec4f* dest, const vec4f* v){
    dest->x *= v->x;
    dest->y *= v->y;
    dest->z *= v->z;
    dest->w *= v->w;
}

MATH_FORCE_INLINE void vec4f_div(vec4f* dest, const vec4f* v){
    math_assert(v->x != 0.0f);
    math_assert(v->y != 0.0f);
    math_assert(v->z != 0.0f);
    math_assert(v->w != 0.0f);
    dest->x /= v->x;
    dest->y /= v->y;
    dest->z /= v->z;
    dest->w /= v->w;
}

MATH_FORCE_INLINE void vec4f_mul_s(vec4f* dest, const float scalar){
    dest->x *= scalar;
    dest->y *= scalar;
    dest->z *= scalar;
    dest->w *= scalar;
}

MATH_FORCE_INLINE void vec4f_div_s(vec4f* dest, const float scalar){
    math_assert(scalar != 0.0f);
    dest->x /= scalar;
    dest->y /= scalar;
    dest->z /= scalar;
    dest->w /= scalar;
}

MATH_FORCE_INLINE void vec4f_mul_add(vec4f* dest, const vec4f* a, const vec4f* b){
    dest->x += a->x * b->x;
    dest->y += a->y * b->y;
    dest->z += a->z * b->z;
    dest->w += a->w * b->w;
}

MATH_FORCE_INLINE void vec4f_mul_add_s(vec4f* dest, const float scalar, const vec4f* v){
    dest->x += v->x * scalar;
    dest->y += v->y * scalar;
    dest->z += v->z * scalar;
    dest->w += v->w * scalar;
}

MATH_FORCE_INLINE float vec4f_dot( const vec4f* a, const vec4f* b ){
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

MATH_FORCE_INLINE float vec4f_mag(const vec4f* v){
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z + v->w * v->w);
}

MATH_FORCE_INLINE float vec4f_mag_sq(const vec4f* v){
    return vec4f_dot(v, v);
}

MATH_FORCE_INLINE void vec4f_norm(vec4f* dest){
    float magnitude = vec4f_mag(dest);
    math_assert(magnitude != 0.0f);
    vec4f_div_s(dest, magnitude);
}

MATH_FORCE_INLINE void vec4f_scale(vec4f* dest, const vec4f* v, const float scalar){
    dest->x = v->x * scalar;
    dest->y = v->y * scalar;
    dest->z = v->z * scalar;
    dest->w = v->w * scalar;
}
//=============================================================================

#endif //VEC4_H
