#ifndef GFX_TYPES_H
#define GFX_TYPES_H

//===INCLUDES==================================================================
#include <kah_math/vec3.h>
#include <kah_math/mat4.h>

#include <stdint.h>
//=============================================================================

// NOTE: API Agnostic types.

//===POOL_TYPES================================================================
typedef uint32_t TransformHandle;

struct LitEntity {
    uint32_t transformIndex;
    uint32_t meshIndex;
    uint32_t materialIndex;
#if KAH_DEBUG
    char debug_name[128]; //TODO: replace with gfx string arena.
#endif //BEET_DEBUG
} typedef LitEntity;
typedef uint32_t LitEntityHandle;

struct CameraEntity {
    uint32_t transformIndex;
    uint32_t cameraIndex;
#if KAH_DEBUG
    char debug_name[128]; // //TODO: replace with gfx string arena.
#endif //BEET_DEBUG
} typedef CameraEntity;
typedef uint32_t CameraEntityHandle;

struct Camera {
    float fov;
    float zNear;
    float zFar;
}typedef Camera;
typedef uint32_t CameraHandle;

struct LitMaterial {
    uint32_t albedoImageIndex;
} typedef LitMaterial;
typedef uint32_t LitMaterialHandle;

struct SceneUBO {
    mat4f projection;
    mat4f view;
    vec3f position;
    float unused_0;
} typedef SceneUBO;
//=============================================================================

#endif //GFX_TYPES_H
