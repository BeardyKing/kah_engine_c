#ifndef MAT4_H
#define MAT4_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>
#include <kah_math/vec3.h>
#include <kah_math/vec4.h>
//=============================================================================

//===API=======================================================================
//void  mat4f_translate                         (   mat4f* dest,    const vec3f* v                                                                  )
//void  mat4f_scale                             (   mat4f* dest,    const vec3f* v                                                                  )
//void  mat4f_scale_s                           (   mat4f* dest,    const float scalar                                                              )
//void  mat4f_mul                               (   mat4f* dest,    const mat4f* m1                                                                 )
//void  mat4f_mul2                              (   mat4f* dest,    const mat4f* m1,    const mat4f* m2                                             )
//void  mat4f_rotate_axis                       (   mat4f* dest,    const vec3f* axis,  const float angle                                           )
//void  mat4f_perspective_rh_zero_to_one        (   mat4f* dest,    const float fovy,   const float aspect, const float zNear,  const float zFar    )
//void  mat4f_perspective_rh_minus_one_to_one   (   mat4f* dest,    const float fovy,   const float aspect, const float zNear,  const float zFar    )
//=============================================================================

//===PUBLIC_STRUCTS============================================================
struct mat4f{
    vec4f col[4];
} typedef mat4f;

#define MAT4F_IDENTITY  (mat4f){ (vec4f){1.0f, 0.0f, 0.0f, 0.0f}, (vec4f){0.0f, 1.0f, 0.0f, 0.0f}, (vec4f){0.0f, 0.0f, 1.0f, 0.0f}, (vec4f){0.0f, 0.0f, 0.0f, 1.0f}, }
#define MAT4F_ZERO      (mat4f){ (vec4f){0.0f, 0.0f, 0.0f, 0.0f}, (vec4f){0.0f, 0.0f, 0.0f, 0.0f}, (vec4f){0.0f, 0.0f, 0.0f, 0.0f}, (vec4f){0.0f, 0.0f, 0.0f, 0.0f}, }
#define KAH_FLIP_Y_PROJ 1
//=============================================================================

//===INLINE====================================================================
MATH_FORCE_INLINE void mat4f_translate(mat4f* dest, const vec3f* v){
    vec4f_mul_add_s(&dest->col[3], v->x, &dest->col[0]);
    vec4f_mul_add_s(&dest->col[3], v->y, &dest->col[1]);
    vec4f_mul_add_s(&dest->col[3], v->z, &dest->col[2]);
}

MATH_FORCE_INLINE void mat4f_scale(mat4f* dest, const vec3f* v){
    vec4f_mul_s(&dest->col[0], v->x);
    vec4f_mul_s(&dest->col[1], v->y);
    vec4f_mul_s(&dest->col[2], v->z);
}

MATH_FORCE_INLINE void mat4f_scale_s(mat4f* dest, const float scalar){
    vec4f_mul_s(&dest->col[0], scalar);
    vec4f_mul_s(&dest->col[1], scalar);
    vec4f_mul_s(&dest->col[2], scalar);
}

MATH_FORCE_INLINE void mat4f_mul(mat4f* dest, const mat4f* m1){
    float a00 = dest->col[0].x; float a01 = dest->col[0].y; float a02 = dest->col[0].z; float a03 = dest->col[0].w;
    float a10 = dest->col[1].x; float a11 = dest->col[1].y; float a12 = dest->col[1].z; float a13 = dest->col[1].w;
    float a20 = dest->col[2].x; float a21 = dest->col[2].y; float a22 = dest->col[2].z; float a23 = dest->col[2].w;
    float a30 = dest->col[3].x; float a31 = dest->col[3].y; float a32 = dest->col[3].z; float a33 = dest->col[3].w;

    float b00 = m1->col[0].x; float b01 = m1->col[0].y; float b02 = m1->col[0].z; float b03 = m1->col[0].w;
    float b10 = m1->col[1].x; float b11 = m1->col[1].y; float b12 = m1->col[1].z; float b13 = m1->col[1].w;
    float b20 = m1->col[2].x; float b21 = m1->col[2].y; float b22 = m1->col[2].z; float b23 = m1->col[2].w;
    float b30 = m1->col[3].x; float b31 = m1->col[3].y; float b32 = m1->col[3].z; float b33 = m1->col[3].w;

    dest->col[0].x = a00*b00 + a10*b01 + a20*b02 + a30*b03;
    dest->col[0].y = a01*b00 + a11*b01 + a21*b02 + a31*b03;
    dest->col[0].z = a02*b00 + a12*b01 + a22*b02 + a32*b03;
    dest->col[0].w = a03*b00 + a13*b01 + a23*b02 + a33*b03;

    dest->col[1].x = a00*b10 + a10*b11 + a20*b12 + a30*b13;
    dest->col[1].y = a01*b10 + a11*b11 + a21*b12 + a31*b13;
    dest->col[1].z = a02*b10 + a12*b11 + a22*b12 + a32*b13;
    dest->col[1].w = a03*b10 + a13*b11 + a23*b12 + a33*b13;

    dest->col[2].x = a00*b20 + a10*b21 + a20*b22 + a30*b23;
    dest->col[2].y = a01*b20 + a11*b21 + a21*b22 + a31*b23;
    dest->col[2].z = a02*b20 + a12*b21 + a22*b22 + a32*b23;
    dest->col[2].w = a03*b20 + a13*b21 + a23*b22 + a33*b23;

    dest->col[3].x = a00*b30 + a10*b31 + a20*b32 + a30*b33;
    dest->col[3].y = a01*b30 + a11*b31 + a21*b32 + a31*b33;
    dest->col[3].z = a02*b30 + a12*b31 + a22*b32 + a32*b33;
    dest->col[3].w = a03*b30 + a13*b31 + a23*b32 + a33*b33;
}

MATH_FORCE_INLINE void mat4f_mul2(mat4f* dest, const mat4f* m1, const mat4f* m2){
    float a00 = m1->col[0].x; float a01 = m1->col[0].y; float a02 = m1->col[0].z; float a03 = m1->col[0].w;
    float a10 = m1->col[1].x; float a11 = m1->col[1].y; float a12 = m1->col[1].z; float a13 = m1->col[1].w;
    float a20 = m1->col[2].x; float a21 = m1->col[2].y; float a22 = m1->col[2].z; float a23 = m1->col[2].w;
    float a30 = m1->col[3].x; float a31 = m1->col[3].y; float a32 = m1->col[3].z; float a33 = m1->col[3].w;

    float b00 = m2->col[0].x; float b01 = m2->col[0].y; float b02 = m2->col[0].z; float b03 = m2->col[0].w;
    float b10 = m2->col[1].x; float b11 = m2->col[1].y; float b12 = m2->col[1].z; float b13 = m2->col[1].w;
    float b20 = m2->col[2].x; float b21 = m2->col[2].y; float b22 = m2->col[2].z; float b23 = m2->col[2].w;
    float b30 = m2->col[3].x; float b31 = m2->col[3].y; float b32 = m2->col[3].z; float b33 = m2->col[3].w;

    dest->col[0].x = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
    dest->col[0].y = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
    dest->col[0].z = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
    dest->col[0].w = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;

    dest->col[1].x = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
    dest->col[1].y = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
    dest->col[1].z = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
    dest->col[1].w = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;

    dest->col[2].x = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
    dest->col[2].y = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
    dest->col[2].z = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
    dest->col[2].w = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;

    dest->col[3].x = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
    dest->col[3].y = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
    dest->col[3].z = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
    dest->col[3].w = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;
}

MATH_FORCE_INLINE void mat4f_rotate_axis(mat4f* dest, const vec3f* axis, const float angle){
    // TODO: replace with math_assert(vec3f_mag(axis) != 0.0f);
    if(vec3f_mag(axis) == 0.0f){
        return;
    }
    vec3f n = *axis;
    vec3f_norm(&n);

    const float x = n.x;
    const float y = n.y;
    const float z = n.z;

    const float c = cosf(angle);
    const float s = sinf(angle);
    const float t = 1.0f - c;

    const mat4f r = (mat4f){
        (vec4f){ t*x*x + c,     t*x*y + s*z, t*x*z - s*y, 0.0f },
        (vec4f){ t*x*y - s*z,   t*y*y + c,   t*y*z + s*x, 0.0f },
        (vec4f){ t*x*z + s*y,   t*y*z - s*x, t*z*z + c,   0.0f },
        (vec4f){ 0.0f,          0.0f,        0.0f,        1.0f }
    };

    mat4f_mul(dest, &r);
}

MATH_FORCE_INLINE void mat4f_perspective_rh_zero_to_one( mat4f* dest, const float fovy, const float aspect, const float zNear, const float zFar){
    // TODO: replace with math_assert
    if(fabsf(aspect) <= 1e-6f){
        return;
    }

    const float tanHalfFovy = tan(fovy / 2.0f);

    *dest = MAT4F_ZERO;
    dest->col[0].x = 1.0f / (aspect * tanHalfFovy);
    dest->col[1].y = (KAH_FLIP_Y_PROJ ? -1.0f : 1.0f) / (tanHalfFovy);
    dest->col[2].z = zFar / (zNear - zFar);
    dest->col[2].w = - (1.0f);
    dest->col[3].z = - (zFar * zNear) / (zFar - zNear);
}

MATH_FORCE_INLINE void mat4f_perspective_rh_minus_one_to_one( mat4f* dest, const float fovy, const float aspect, const float zNear, const float zFar){
    // TODO: replace with math_assert
    if(fabsf(aspect) <= 1e-6f){
        return;
    }

    const float tanHalfFovy = tan(fovy / 2.0f);

    *dest = MAT4F_ZERO;
    dest->col[0].x = 1.0f / (aspect * tanHalfFovy);
    dest->col[1].y = (KAH_FLIP_Y_PROJ ? -1.0f : 1.0f) / (tanHalfFovy);
    dest->col[2].z = - (zFar + zNear) / (zFar - zNear);
    dest->col[2].w = - 1.0f;
    dest->col[3].z = - (2.0f * zFar * zNear) / (zFar - zNear);
}



#endif //MAT4_H
