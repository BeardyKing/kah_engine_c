#ifndef TRANSFORM_H
#define TRANSFORM_H

//===INCLUDES==================================================================
#include <kah_math/defines.h>
#include <kah_math/utils.h>
#include <kah_math/vec3.h>
#include <kah_math/mat4.h>
#include <kah_math/quat.h>
//=============================================================================

//===API=======================================================================
//mat4f transform_model_matrix              (   const Transform* t  )
//mat4f transform_model_matrix_quat_cast    (   const Transform* t  )
//=============================================================================

//===PUBLIC_STRUCTS============================================================
struct Transform {
    vec3f position;
    vec3f rotation;
    vec3f scale;
} typedef Transform;
//=============================================================================

//===INLINE====================================================================
MATH_FORCE_INLINE Transform transform_default(){
    return (Transform){
        .position = (vec3f){.x = 0, .y = 0, .z = 0},
        .rotation = (vec3f){.x = 0, .y = 0, .z = 0},
        .scale =    (vec3f){.x = 1, .y = 1, .z = 1},
    };
}

MATH_FORCE_INLINE mat4f transform_model_matrix(const Transform* t){
    mat4f out = MAT4F_IDENTITY;
    mat4f_translate(&out, &t->position);
    mat4f_rotate_axis(&out, &VEC3F_WORLD_RIGHT, as_radians_f(t->rotation.x));
    mat4f_rotate_axis(&out, &VEC3F_WORLD_UP, as_radians_f(t->rotation.y));
    mat4f_rotate_axis(&out, &VEC3F_WORLD_FORWARD, as_radians_f(t->rotation.z));
    mat4f_scale(&out, &t->scale);
    return out;
}

MATH_FORCE_INLINE mat4f transform_model_matrix_quat_cast(const Transform* t){
    //TODO: When transform.rotation becomes a quat this should become the default, instead of using angle axis mat4f, i.e. just a quat_to_mat4f.
    mat4f out = MAT4F_IDENTITY;
    {
        mat4f_translate(&out, &t->position);

        quat q = QUAT_IDENTITY;
        quat_from_euler(&q, &(vec3f){.x = as_radians_f(t->rotation.x), .y = as_radians_f(t->rotation.y), .z = as_radians_f(t->rotation.z)});
        quat_to_mat4f(&out, &q);

        mat4f_scale(&out, &t->scale);
    }
    return out;
}
//=============================================================================

#endif //TRANSFORM_H
