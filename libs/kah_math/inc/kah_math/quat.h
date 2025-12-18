#ifndef QUAT_H
#define QUAT_H

//===INCLUDES==================================================================
#include <kah_math/mat4.h>
//=============================================================================

//===API=======================================================================
// void quat_from_euler (   quat* dest,     const vec3f* euler  )
// void quat_to_euler   (   vec3f* dest,    const quat* q       )
// void quat_to_mat4f   (   mat4f* dest,    const quat* q       )
// void quat_conjugate  (   quat* dest                          )
// void quat_conjugate2 (   quat* dest,     const quat* q       )
//=============================================================================

//===PUBLIC_STRUCTS============================================================
typedef vec4f quat;

#define QUAT_IDENTITY   ((vec4f){ .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f, })
#define QUAT_ZERO       ((vec4f){ .x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f, })
//=============================================================================

//===INLINE====================================================================
MATH_FORCE_INLINE void quat_from_euler(quat* dest, const vec3f* euler){
    const vec3f c = (vec3f){.x = cosf(euler->x * 0.5), cosf(euler->y * 0.5), cosf(euler->z * 0.5)};
    const vec3f s = (vec3f){.x = sinf(euler->x * 0.5), sinf(euler->y * 0.5), sinf(euler->z * 0.5)};

    dest->w = c.x * c.y * c.z + s.x * s.y * s.z;
    dest->x = s.x * c.y * c.z - c.x * s.y * s.z;
    dest->y = c.x * s.y * c.z + s.x * c.y * s.z;
    dest->z = c.x * c.y * s.z - s.x * s.y * c.z;
}

MATH_FORCE_INLINE void quat_to_euler(vec3f* dest, const quat* q){
    const float sinx_cosp = 2 * (q->w * q->x + q->y * q->z);
    const float cosx_cosp = 1 - 2 * (q->x * q->x + q->y * q->y);
    dest->x = atan2(sinx_cosp, cosx_cosp);

    const float siny = sqrt(1 + 2 * (q->w * q->y - q->x * q->z));
    const float cosy = sqrt(1 - 2 * (q->w * q->y - q->x * q->z));
    dest->y = 2 * atan2(siny, cosy) - KAH_PI_FLOAT / 2;

    const float sinz_cosp = 2 * (q->w * q->z + q->x * q->y);
    const float cosz_cosp = 1 - 2 * (q->y * q->y + q->z * q->z);
    dest->z = atan2(sinz_cosp, cosz_cosp);
}

MATH_FORCE_INLINE void quat_to_mat4f(mat4f* dest, const quat* q){
    //.w & col[4] left untouched
    const float qxx = q->x * q->x;
    const float qyy = q->y * q->y;
    const float qzz = q->z * q->z;
    const float qxz = q->x * q->z;
    const float qxy = q->x * q->y;
    const float qyz = q->y * q->z;
    const float qwx = q->w * q->x;
    const float qwy = q->w * q->y;
    const float qwz = q->w * q->z;

    dest->col[0].x = 1.0f - 2.0f * (qyy +  qzz);
    dest->col[0].y = 2.0f * (qxy + qwz);
    dest->col[0].z = 2.0f * (qxz - qwy);

    dest->col[1].x = 2.0f * (qxy - qwz);
    dest->col[1].y = 1.0f - 2.0f * (qxx +  qzz);
    dest->col[1].z = 2.0f * (qyz + qwx);

    dest->col[2].x = 2.0f * (qxz + qwy);
    dest->col[2].y = 2.0f * (qyz - qwx);
    dest->col[2].z = 1.0f - 2.0f * (qxx +  qyy);
}

MATH_FORCE_INLINE void quat_conjugate(quat* dest){
    dest->x = -dest->x;
    dest->y = -dest->y;
    dest->z = -dest->z;
}

MATH_FORCE_INLINE void quat_conjugate2(quat* dest, const quat* q){
    *dest = (quat){
        .x = -q->x,
        .y = -q->y,
        .z = -q->z,
        .w = q->w,
    };
}
//=============================================================================

#endif //QUAT_H
