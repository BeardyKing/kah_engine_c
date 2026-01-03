#include <kah_core/defines.h>
#if CHECK_FEATURE(FEATURE_PLATFORM_LINUX)

#include <kah_core/assert.h>
#include <kah_core/window.h>
#include <kah_core/input_types.h>
#include <kah_core/input.h>

#include <kah_math/vec2.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/keysym.h>

//===INTERNAL_STRUCTS==========================================================
#define KAH_MAX_WINDOW_SIZE_X 16384
#define KAH_MAX_WINDOW_SIZE_Y 16384
#define KAH_MIN_WINDOW_SIZE_X 256
#define KAH_MIN_WINDOW_SIZE_Y 256

typedef struct XLibHandles // used in kah_gfx/vulkan/gfx_vulkan_surface_linux.c
{
    Display* display;
    Window window;
} XLibHandles;

struct WindowInfo{
    const char* applicationName;
    const char* titleName;

    int32_t width;
    int32_t height;
    int32_t posX;
    int32_t posY;
    bool shouldWindowClose;

    InputCursorState currentCursorState;

    vec2i lockedCursorPosition;
    vec2i virtualCursorPosition;
    vec2i lastPosition;
    bool cursorOverWindow;
    XLibHandles xLibHandles;
    int screen;

    XEvent event;
}typedef WindowInfo;

static WindowInfo s_windowInfo = {};
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static void cursor_hide(Display* display, Window window){
    XFixesHideCursor(display, window);
}

static void cursor_show(Display* display, Window window){
    XFixesShowCursor(display, window);
}

static void cursor_lock(Display* display, Window window){
    XGrabPointer(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window,
                 True,ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, s_windowInfo.xLibHandles.window, None, CurrentTime
    );
}

InputKeyCode x11_to_kah_keycode(KeySym keysym){
    //switch to indirection table
    switch (keysym)
    {
    case XK_space: return KEYCODE_SPACE;
    case XK_Return: return KEYCODE_ENTER;
    case XK_Escape: return KEYCODE_ESCAPE;
    case XK_BackSpace: return KEYCODE_BACKSPACE;
    case XK_Tab: return KEYCODE_TAB;
    case XK_Shift_L: return KEYCODE_LEFTSHIFT;
    case XK_Shift_R: return KEYCODE_RIGHTSHIFT;
    case XK_Control_L: return KEYCODE_LEFTCONTROL;
    case XK_Control_R: return KEYCODE_RIGHTCONTROL;
    case XK_Alt_L: return KEYCODE_ALT;
    case XK_Alt_R: return KEYCODE_MENU;
    case XK_Meta_L: return KEYCODE_LEFTSUPER;
    case XK_Meta_R: return KEYCODE_RIGHTSUPER;

    case XK_0: return KEYCODE_N0;
    case XK_1: return KEYCODE_N1;
    case XK_2: return KEYCODE_N2;
    case XK_3: return KEYCODE_N3;
    case XK_4: return KEYCODE_N4;
    case XK_5: return KEYCODE_N5;
    case XK_6: return KEYCODE_N6;
    case XK_7: return KEYCODE_N7;
    case XK_8: return KEYCODE_N8;
    case XK_9: return KEYCODE_N9;

    case XK_A:
    case XK_a: return KEYCODE_A;
    case XK_B:
    case XK_b: return KEYCODE_B;
    case XK_C:
    case XK_c: return KEYCODE_C;
    case XK_D:
    case XK_d: return KEYCODE_D;
    case XK_E:
    case XK_e: return KEYCODE_E;
    case XK_F:
    case XK_f: return KEYCODE_F;
    case XK_G:
    case XK_g: return KEYCODE_G;
    case XK_H:
    case XK_h: return KEYCODE_H;
    case XK_I:
    case XK_i: return KEYCODE_I;
    case XK_J:
    case XK_j: return KEYCODE_J;
    case XK_K:
    case XK_k: return KEYCODE_K;
    case XK_L:
    case XK_l: return KEYCODE_L;
    case XK_M:
    case XK_m: return KEYCODE_M;
    case XK_N:
    case XK_n: return KEYCODE_N;
    case XK_O:
    case XK_o: return KEYCODE_O;
    case XK_P:
    case XK_p: return KEYCODE_P;
    case XK_Q:
    case XK_q: return KEYCODE_Q;
    case XK_R:
    case XK_r: return KEYCODE_R;
    case XK_S:
    case XK_s: return KEYCODE_S;
    case XK_T:
    case XK_t: return KEYCODE_T;
    case XK_U:
    case XK_u: return KEYCODE_U;
    case XK_V:
    case XK_v: return KEYCODE_V;
    case XK_W:
    case XK_w: return KEYCODE_W;
    case XK_X:
    case XK_x: return KEYCODE_X;
    case XK_Y:
    case XK_y: return KEYCODE_Y;
    case XK_Z:
    case XK_z: return KEYCODE_Z;

    case XK_Up: return KEYCODE_UP;
    case XK_Down: return KEYCODE_DOWN;
    case XK_Left: return KEYCODE_LEFT;
    case XK_Right: return KEYCODE_RIGHT;

    case XK_F1: return KEYCODE_F1;
    case XK_F2: return KEYCODE_F2;
    case XK_F3: return KEYCODE_F3;
    case XK_F4: return KEYCODE_F4;
    case XK_F5: return KEYCODE_F5;
    case XK_F6: return KEYCODE_F6;
    case XK_F7: return KEYCODE_F7;
    case XK_F8: return KEYCODE_F8;
    case XK_F9: return KEYCODE_F9;
    case XK_F10: return KEYCODE_F10;
    case XK_F11: return KEYCODE_F11;
    case XK_F12: return KEYCODE_F12;

    default: return KEYCODE_UNKNOWN;
    }
}

typedef void (*EventCallback)(XEvent* event);
static EventCallback s_xEventCallback = nullptr;

void window_set_procedure_callback_func(void* procCallback)
{
    s_xEventCallback = (EventCallback)procCallback;
}

static void x_event_callback(XEvent* event){
    if (s_xEventCallback){
        s_xEventCallback(event);
    }
}

static void window_poll(){
    while (XPending(s_windowInfo.xLibHandles.display) > 0){
        XNextEvent(s_windowInfo.xLibHandles.display, &s_windowInfo.event);
        x_event_callback(&s_windowInfo.event);
        switch (s_windowInfo.event.type)
        {
        case Expose:
            break;

        case KeyPress:
            {
                int key = XkbKeycodeToKeysym(s_windowInfo.xLibHandles.display, s_windowInfo.event.xkey.keycode, 0, 0);
                InputKeyCode keyCode = x11_to_kah_keycode(key);
                input_key_down((int32_t)keyCode);

                if (key == XK_Q){
                    if (s_windowInfo.currentCursorState != CURSOR_LOCKED){
                        window_set_cursor(CURSOR_LOCKED);
                        window_set_cursor_lock_position(s_windowInfo.virtualCursorPosition);
                    }
                    else{
                        window_set_cursor(CURSOR_NORMAL);
                    }
                }
                break;
            }

        case KeyRelease:
            {
                int key = XkbKeycodeToKeysym(s_windowInfo.xLibHandles.display, s_windowInfo.event.xkey.keycode, 0, 0);
                InputKeyCode keyCode = x11_to_kah_keycode(key);
                input_key_up((int32_t)keyCode);
                break;
            }

        case ConfigureNotify:
            s_windowInfo.width = s_windowInfo.event.xconfigure.width;
            s_windowInfo.height = s_windowInfo.event.xconfigure.height;
            break;

        case MotionNotify:
            {
                vec2i delta;
                delta.x = s_windowInfo.event.xmotion.x - s_windowInfo.lastPosition.x;
                delta.y = s_windowInfo.event.xmotion.y - s_windowInfo.lastPosition.y;
                s_windowInfo.virtualCursorPosition.x += delta.x;
                s_windowInfo.virtualCursorPosition.y += delta.y;

                input_mouse_move(s_windowInfo.virtualCursorPosition.x, s_windowInfo.virtualCursorPosition.y);
                input_mouse_windowed_position(s_windowInfo.event.xmotion.x, s_windowInfo.event.xmotion.y);

                if (s_windowInfo.currentCursorState == CURSOR_HIDDEN_LOCKED_LOCK_MOUSE_POS){
                    s_windowInfo.lastPosition.x = s_windowInfo.lockedCursorPosition.x;
                    s_windowInfo.lastPosition.y = s_windowInfo.lockedCursorPosition.y;
                    XWarpPointer(s_windowInfo.xLibHandles.display, None, s_windowInfo.xLibHandles.window, 0, 0, 0, 0, s_windowInfo.lastPosition.x, s_windowInfo.lastPosition.y);
                }
                else{
                    s_windowInfo.lastPosition.x = s_windowInfo.event.xmotion.x;
                    s_windowInfo.lastPosition.y = s_windowInfo.event.xmotion.y;
                }
                break;
            }

        case FocusOut:
            window_set_cursor(CURSOR_NORMAL);
            break;

        case ButtonPress:
            {
                int button = s_windowInfo.event.xbutton.button;
                switch (button)
                {
                case Button1:
                    input_mouse_down((int32_t)MOUSE_LEFT);
                    break;
                case Button2:
                    input_mouse_down((int32_t)MOUSE_MIDDLE);
                    break;
                case Button3:
                    input_mouse_down((int32_t)MOUSE_RIGHT);
                    break;
                case Button4: // Scroll up
                    input_mouse_scroll(1);
                    break;
                case Button5: // Scroll down
                    input_mouse_scroll(-1);
                    break;
                default:
                    break;
                }
                break;
            }

        case ButtonRelease:
            {
                int button = s_windowInfo.event.xbutton.button;
                switch (button)
                {
                case Button1:
                    input_mouse_up((int32_t)MOUSE_LEFT);
                    break;
                case Button2:
                    input_mouse_up((int32_t)MOUSE_MIDDLE);
                    break;
                case Button3:
                    input_mouse_up((int32_t)MOUSE_RIGHT);
                    break;
                default:
                    break;
                }
                break;
            }

        case LeaveNotify:
            s_windowInfo.cursorOverWindow = false;
            break;

        case EnterNotify:
            s_windowInfo.cursorOverWindow = true;
            break;

        default:
            break;
        }
    }
}

//======================================================================================================================

//===API================================================================================================================
void window_update(){
    window_poll();
}

bool window_is_open(){
    return !s_windowInfo.shouldWindowClose;
}

bool window_is_cursor_over_window(){
    return s_windowInfo.cursorOverWindow;
}

void window_set_cursor_lock_position(const vec2i lockPos){
    s_windowInfo.lockedCursorPosition = (vec2i){
        s_windowInfo.posX + lockPos.x,
        s_windowInfo.posY + lockPos.y
    };
}

void window_set_cursor(InputCursorState state){
    switch (state){
    case CURSOR_NORMAL:
        if (s_windowInfo.currentCursorState == CURSOR_HIDDEN ||
            s_windowInfo.currentCursorState == CURSOR_HIDDEN_LOCKED ||
            s_windowInfo.currentCursorState == CURSOR_HIDDEN_LOCKED_LOCK_MOUSE_POS)
        {
            cursor_show(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        }

        if (s_windowInfo.currentCursorState == CURSOR_HIDDEN_LOCKED_LOCK_MOUSE_POS){
            XWarpPointer(s_windowInfo.xLibHandles.display, None, s_windowInfo.xLibHandles.window, 0, 0, 0, 0, s_windowInfo.lockedCursorPosition.x,
                         s_windowInfo.lockedCursorPosition.y);
        }

        if (s_windowInfo.currentCursorState == CURSOR_LOCKED ||
            s_windowInfo.currentCursorState == CURSOR_HIDDEN_LOCKED ||
            s_windowInfo.currentCursorState == CURSOR_HIDDEN_LOCKED_LOCK_MOUSE_POS)
        {
            XUngrabPointer(s_windowInfo.xLibHandles.display, CurrentTime);
        }

        break;
    case CURSOR_HIDDEN:
        cursor_hide(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        XUngrabPointer(s_windowInfo.xLibHandles.display, CurrentTime);
        break;
    case CURSOR_LOCKED:
        cursor_lock(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        break;
    case CURSOR_HIDDEN_LOCKED:
    case CURSOR_HIDDEN_LOCKED_LOCK_MOUSE_POS:
        cursor_hide(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        cursor_lock(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        break;
    default:
        break;
    }
    s_windowInfo.currentCursorState = state;
}

void* window_get_handle(){
    return &s_windowInfo.xLibHandles;
}

//=============================================================================

//===INIT/SHUTDOWN=============================================================
void window_create(const char* windowTitle, const vec2i windowSize, const vec2i windowPosition) {
    s_windowInfo.applicationName = "kah engine (linux)";
    s_windowInfo.titleName = windowTitle;
    s_windowInfo.width = windowSize.x;
    s_windowInfo.height = windowSize.y;
    s_windowInfo.posX = windowPosition.x; // Corrected: use x for position
    s_windowInfo.posY = windowPosition.y;

    s_windowInfo.xLibHandles.display = XOpenDisplay(nullptr);
    if (s_windowInfo.xLibHandles.display == nullptr){
        // TODO: ADD ASSERT
        return;
    }
    s_windowInfo.screen = DefaultScreen(s_windowInfo.xLibHandles.display);
    s_windowInfo.xLibHandles.window = XCreateSimpleWindow(
        s_windowInfo.xLibHandles.display,
        RootWindow(s_windowInfo.xLibHandles.display, s_windowInfo.screen),
        s_windowInfo.posX, s_windowInfo.posY,
        s_windowInfo.width, s_windowInfo.height,
        1,
        BlackPixel(s_windowInfo.xLibHandles.display, s_windowInfo.screen),
        WhitePixel(s_windowInfo.xLibHandles.display, s_windowInfo.screen)
    );

    XSizeHints hints = {
        .flags = PMinSize | PMaxSize,
        .min_width = KAH_MIN_WINDOW_SIZE_X,
        .min_height = KAH_MIN_WINDOW_SIZE_Y,
        .max_width = KAH_MAX_WINDOW_SIZE_X,
        .max_height = KAH_MAX_WINDOW_SIZE_Y,
    };
    XSetNormalHints(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window, &hints);
    XStoreName(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window, s_windowInfo.titleName);

    XSelectInput(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window,
                 ExposureMask | KeyPressMask | KeyReleaseMask |
                 StructureNotifyMask | PointerMotionMask |
                 ButtonPressMask | ButtonReleaseMask |
                 EnterWindowMask | LeaveWindowMask);

    XMapWindow(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
}

void window_cleanup(){
    XDestroyWindow(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
    XCloseDisplay(s_windowInfo.xLibHandles.display);
    s_windowInfo = (WindowInfo){};
}
//=============================================================================
#endif