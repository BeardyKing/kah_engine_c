#ifndef WINDOW_H
#define WINDOW_H

//===INCLUDES==================================================================
#include <kah_math/vec2.h>
#include <kah_core/input_types.h>

#include <stdbool.h>
//=============================================================================

#define KAH_MAX_WINDOW_TITLE_SIZE 256
#define KAH_WINDOW_POSITION_CENTERED (vec2i){0xFFFFFFFF, 0xFFFFFFFF}

//===API=======================================================================
bool  window_is_open();
void  window_update();
void  window_set_cursor(InputCursorState state);
void  window_set_cursor_lock_position(vec2i lockPos);
bool  window_is_cursor_over_window();
void* window_get_handle();

//CALLBACK: WIN32: LRESULT (*)(HWND, UINT, WPARAM, LPARAM
//CALLBACK: LINUX  : typedef void (*EventCallback)(XEvent* event);
void window_set_procedure_callback_func(void *procCallback);
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void window_create(const char* windowTitle, vec2i windowSize, vec2i windowPosition);
void window_cleanup();
//=============================================================================

#endif //WINDOW_H
