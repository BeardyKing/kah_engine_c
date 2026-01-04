#ifndef GFX_INTERFACE_H
#define GFX_INTERFACE_H

//===INCLUDES==================================================================
#include <kah_gfx/gfx_types.h>
//=============================================================================

//===API=======================================================================
void gfx_update();
bool gfx_has_drawable_surface();

void gfx_camera_main_set_active(CameraEntityHandle camHandle);
CameraEntityHandle gfx_camera_main_get_active();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_create(void *windowHandle);
void gfx_cleanup();
//=============================================================================

#endif //GFX_INTERFACE_H
