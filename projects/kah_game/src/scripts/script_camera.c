//===INCLUDES==================================================================
#include <client/scripts/script_camera.h>

#include <kah_core/input.h>
#include <kah_core/window.h>
#include <kah_core/time.h>

#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/gfx_pool.h>

#include <kah_math/transform.h>
//=============================================================================

//===API=======================================================================
void script_camera_update(){
    CameraEntityHandle camEntityHandle = gfx_camera_main_get_active();
    CameraEntity* camEntity = gfx_pool_camera_entity_get(camEntityHandle);
    Transform *transform = gfx_pool_transform_get(camEntity->transformIndex);


    if (input_is_mouse_pressed(MOUSE_RIGHT)) {
        window_set_cursor(CURSOR_HIDDEN_LOCKED_LOCK_MOUSE_POS);
        window_set_cursor_lock_position(input_mouse_position());
    }

    if (input_is_mouse_released(MOUSE_RIGHT)) {
        window_set_cursor(CURSOR_NORMAL);
    }

    constexpr float INPUT_EPSILON = 0.125f;
    constexpr float ROTATION_CLAMP = KAH_HALF_PI_FLOAT - INPUT_EPSILON;

    if (input_is_mouse_down(MOUSE_RIGHT)) {
        const vec2f delta = input_mouse_delta_raw();
        const float mouseSpeed = 0.002f;
        transform->rotation.y += (-delta.x * mouseSpeed);
        transform->rotation.x += (-delta.y * mouseSpeed);
        transform->rotation.x = clamp_f32(transform->rotation.x, -ROTATION_CLAMP, ROTATION_CLAMP);

        vec3f camForward = VEC3F_ZERO;
        vec3f camRight = VEC3F_ZERO;
        quat cameraRotation = QUAT_ZERO;
        quat_from_euler(&cameraRotation, &transform->rotation);
        quat_rotate_vec3f(&camForward, &cameraRotation, &VEC3F_WORLD_FORWARD);
        quat_rotate_vec3f(&camRight, &cameraRotation, &VEC3F_WORLD_RIGHT);

        vec3f moveDirection = VEC3F_ZERO;
        float moveSpeed = 5.0f;
        const float speedUpScalar = 80.0f;
        const float speedDownScalar = 0.1f;
        //TODO: Add axis API.
        if (input_is_key_down(KEYCODE_W)) {
            vec3f_add(&moveDirection, &camForward);
        }
        if (input_is_key_down(KEYCODE_S)) {
            vec3f_sub(&moveDirection, &camForward);
        }
        if (input_is_key_down(KEYCODE_A)) {
            vec3f_add(&moveDirection, &camRight);
        }
        if (input_is_key_down(KEYCODE_D)) {
            vec3f_sub(&moveDirection, &camRight);
        }
        if (input_is_key_down(KEYCODE_R)) {
            vec3f_add(&moveDirection, &VEC3F_WORLD_UP);
        }
        if (input_is_key_down(KEYCODE_F)) {
            vec3f_sub(&moveDirection, &VEC3F_WORLD_UP);
        }
        if (input_is_key_down(KEYCODE_SHIFT)) {
            moveSpeed *= speedUpScalar;
        }
        if (input_is_key_down(KEYCODE_CONTROL)) {
            moveSpeed *= speedDownScalar;
        }

        vec3f_mul_s(&moveDirection, moveSpeed);
        vec3f_mul_s(&moveDirection, time_delta_f());
        vec3f_add(&transform->position, &moveDirection);
    }

}
//=============================================================================
