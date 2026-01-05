#ifndef WIDGETS_COMMON_H
#define WIDGETS_COMMON_H

//===INCLUDES==================================================================
#include <kah_gfx/gfx_types.h>
#include <kah_math/transform.h>
#include <kah_math/vec3.h>
//=============================================================================

//===API=======================================================================
bool draw_vec3_control(const char *label, vec3f *values, float resetValue, float columnWidth, float minDrag);
bool widget_draw_camera(Camera *camera);
bool widget_draw_transform(Transform *transform);
//=============================================================================

#endif //WIDGETS_COMMON_H
