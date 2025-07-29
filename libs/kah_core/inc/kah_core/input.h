#ifndef INPUT_H
#define INPUT_H

//===INCLUDES==================================================================
#include <kah_core/input_types.h>
#include <kah_math/vec2.h>

#include <stdbool.h>
//=============================================================================

//===API=======================================================================
void input_update();

void input_set_time(double time);
bool input_is_key_pressed(InputKeyCode key);
bool input_is_key_released(InputKeyCode key);
bool input_is_key_down(InputKeyCode key);
float input_key_down_time(InputKeyCode key);

vec2f input_mouse_delta();
vec2f input_mouse_delta_raw();
vec2i input_mouse_position();

bool input_is_mouse_down(InputMouseButton button);
bool input_is_mouse_pressed(InputMouseButton button);
bool input_is_mouse_released(InputMouseButton key);
float input_mouse_down_time(InputMouseButton button);

float input_mouse_scroll_delta_raw();
float input_mouse_scroll_delta();

void input_key_down(int32_t asciiKeyCode);
void input_key_up(int32_t asciiKeyCode);

void input_mouse_down(int32_t keyCode);
void input_mouse_up(int32_t keyCode);

void input_mouse_move(int32_t x, int32_t y);
void input_mouse_scroll(int32_t y);
void input_mouse_windowed_position(int32_t x, int32_t y);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void input_create();
void input_cleanup();
//=============================================================================


#endif //INPUT_H
