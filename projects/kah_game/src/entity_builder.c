//===INCLUDES==================================================================
#include <client/entity_builder.h>

#include <kah_gfx/gfx_pool.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>

#include <stdio.h>
//=============================================================================

//===INTERNAL==================================================================
static CameraEntityHandle s_primaryCamEnt;
static LitEntityHandle s_defaultCube;

static void entity_primary_camera_create(){
    s_primaryCamEnt = gfx_pool_camera_entity_handle_get_next();
    CameraEntity* camEnt = gfx_pool_camera_entity_get(s_primaryCamEnt);
    camEnt->transformIndex = gfx_pool_transform_handle_get_next();
    camEnt->cameraIndex = gfx_pool_camera_handle_get_next();
    sprintf(camEnt->debug_name, "primary camera");

    Transform* transform = gfx_pool_transform_get(camEnt->transformIndex);
    transform->position = (vec3f){-1.5f, 0.5f, 2.5f};
    transform->rotation = (vec3f){-0.2f, -0.65f, 0.0f};
    transform->scale = (vec3f){1.0f, 1.0f, 1.0f};

    Camera* cam = gfx_pool_camera_get(camEnt->cameraIndex);
    cam->fov = 65.0f;
    cam->zNear = 0.1f;
    cam->zFar = 6000.0f;
}

static void entity_primary_camera_cleanup(){
    CameraEntity* camEnt = gfx_pool_camera_entity_get(s_primaryCamEnt);
    gfx_pool_camera_release(camEnt->cameraIndex);
    gfx_pool_transform_release(camEnt->transformIndex);
    gfx_pool_camera_entity_release(s_primaryCamEnt);
}

static void entity_lit_create(){
    s_defaultCube = gfx_pool_lit_entity_handle_get_next();
    LitEntity* litEnt = gfx_pool_lit_entity_get(s_defaultCube);
    litEnt->transformIndex = gfx_pool_transform_handle_get_next();
    litEnt->meshIndex = gfx_mesh_built_in_cube();
    litEnt->materialIndex = gfx_pool_lit_material_handle_get_next();
    sprintf(litEnt->debug_name, "default cube");

    Transform* litTransform = gfx_pool_transform_get(litEnt->transformIndex);
    *litTransform = transform_default();

    LitMaterial* litMaterial = gfx_pool_lit_material_get(litEnt->materialIndex);
    litMaterial->albedoImageIndex = gfx_texture_built_in_uv_grid();
}

static void entity_lit_cleanup(){
    LitEntity* litEnt = gfx_pool_lit_entity_get(s_defaultCube);
    gfx_pool_transform_release(litEnt->transformIndex);
    gfx_pool_lit_material_release(litEnt->materialIndex);
    gfx_pool_lit_entity_release(s_defaultCube);
    //mesh & image built-ins are released by backend.
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void entity_builder_create(){
    entity_primary_camera_create();
    entity_lit_create();
}

void entity_builder_cleanup(){
    entity_lit_cleanup();
    entity_primary_camera_cleanup();
}
//=============================================================================