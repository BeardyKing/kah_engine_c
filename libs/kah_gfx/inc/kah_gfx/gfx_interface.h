#ifndef GFX_INTERFACE_H
#define GFX_INTERFACE_H

//===API=======================================================================
void gfx_update();
bool gfx_has_drawable_surface();
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_create(void *windowHandle);
void gfx_cleanup();
//=============================================================================

#endif //GFX_INTERFACE_H
