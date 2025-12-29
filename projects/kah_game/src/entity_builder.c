//===INCLUDES==================================================================
#include <client/entity_builder.h>

#include <kah_gfx/gfx_pool.h>

#include <stdio.h>
//=============================================================================

//===INTERNAL==================================================================
static CameraEntityHandle s_primaryCamEnt;

static void primary_camera_create(){
    s_primaryCamEnt = gfx_pool_camera_entity_handle_get_next();
    CameraEntity* camEnt = gfx_pool_camera_entity_get(s_primaryCamEnt);
    camEnt->transformIndex = gfx_pool_transform_handle_get_next();
    camEnt->cameraIndex = gfx_pool_camera_handle_get_next();
#if KAH_DEBUG
    sprintf(camEnt->debug_name, "primary camera");
#endif

    Transform* transform = gfx_pool_transform_get(camEnt->transformIndex);
    transform->position = (vec3f){-1.5f, 0.5f, 2.5f};
    transform->rotation = (vec3f){-0.2f, -0.65f, 0.0f};
    transform->scale = (vec3f){1.0f, 1.0f, 1.0f};

    Camera* cam = gfx_pool_camera_get(camEnt->cameraIndex);
    cam->fov = 65.0f;
    cam->zNear = 0.1f;
    cam->zFar = 6000.0f;
}

static void primary_camera_cleanup(){
    CameraEntity* camEnt = gfx_pool_camera_entity_get(s_primaryCamEnt);
    gfx_pool_camera_release(camEnt->cameraIndex);
    gfx_pool_transform_release(camEnt->transformIndex);
    gfx_pool_camera_entity_release(s_primaryCamEnt);
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void entity_builder_create(){
    primary_camera_create();
}

void entity_builder_cleanup(){
    primary_camera_cleanup();
}
//=============================================================================