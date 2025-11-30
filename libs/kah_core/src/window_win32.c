#include <kah_core/defines.h>
#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
//===INCLUDES==================================================================
#include <stdint.h>

#include <kah_core/assert.h>
#include <kah_core/window.h>
#include <kah_core/input.h>
#include <kah_core/input_types.h>
#include <kah_core/core_cvars.h>

#include <kah_math/vec2.h>

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WindowsX.h>
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
LRESULT (*g_windowProcCallback_Func)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = nullptr;


struct WindowInfo {
    WNDCLASSEX windowClass;
    HWND handle;
    const wchar_t *applicationName;
    const wchar_t *titleName;

    int32_t width;
    int32_t height;
    int32_t posX;
    int32_t posY;
    bool shouldWindowClose;

    InputCursorState currentCursorState;
    vec2i lockedCursorPosition;
    vec2i virtualCursorPosition;
    bool cursorOverWindow;
} typedef WindowInfo;
WindowInfo s_windowInfo = {};
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static LRESULT CALLBACK window_procedure_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (g_windowProcCallback_Func != nullptr) {
        g_windowProcCallback_Func(hwnd, uMsg, wParam, lParam);
    }
    switch (uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            s_windowInfo.shouldWindowClose = true;
            break;
        };
        case WM_PAINT: {
            ValidateRect(hwnd, nullptr);
            break;
        };
        case WM_SIZE:
        case WM_MOVE: {
            RECT rect = (RECT){};
            GetWindowRect(hwnd, &rect);
            s_windowInfo.posX = rect.left;
            s_windowInfo.posY = rect.top;
            s_windowInfo.width = rect.right - rect.left;
            s_windowInfo.height = rect.bottom - rect.top;
            vec2i_cvar_set(g_coreCvars.windowPosition, (vec2i) { .x = s_windowInfo.posX, .y = s_windowInfo.posY });
            vec2i_cvar_set(g_coreCvars.windowSize, (vec2i) { .x = s_windowInfo.width, .y = s_windowInfo.height });
            break;
        }

        case WM_MOUSELEAVE: {
            s_windowInfo.cursorOverWindow = false;
            break;
        }
        case WM_MOUSEHOVER: {
            s_windowInfo.cursorOverWindow = true;
            break;
        }
            //input
        case WM_KEYDOWN: {
            input_key_down((int32_t) wParam);
            break;
        };
        case WM_KEYUP: {
            input_key_up((int32_t) wParam);
            break;
        };
        case WM_SYSKEYDOWN : {
            input_key_down((int32_t) wParam);
            break;
        };
        case WM_SYSKEYUP : {
            input_key_up((int32_t) wParam);
            break;
        };
        case WM_INPUT: {
            unsigned size = sizeof(RAWINPUT);
            static RAWINPUT raw[sizeof(RAWINPUT)];

            GetRawInputData((HRAWINPUT) lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));
            s_windowInfo.virtualCursorPosition.x += raw->data.mouse.lLastX;
            s_windowInfo.virtualCursorPosition.y += raw->data.mouse.lLastY;

            input_mouse_move(s_windowInfo.virtualCursorPosition.x,s_windowInfo.virtualCursorPosition.y);

            //TODO:CORE consider moving other input callbacks to WM_INPUT i.e. WHEEL_DELTA and JOY PADS.
            //          wheel `raw->data.mouse.usButtonData`
            break;
        }
        case WM_MOUSEMOVE: {
            const int32_t x = GET_X_LPARAM(lParam);
            const int32_t y = GET_Y_LPARAM(lParam);
            input_mouse_windowed_position(x, y);
            break;
        }
        case WM_RBUTTONDOWN: {
            input_mouse_down((int32_t)MOUSE_RIGHT);
            break;
        }
        case WM_RBUTTONUP: {
            input_mouse_up((int32_t) MOUSE_RIGHT);
            break;
        }
        case WM_LBUTTONDOWN: {
            input_mouse_down((int32_t) MOUSE_LEFT);
            break;
        }
        case WM_LBUTTONUP: {
            input_mouse_up((int32_t) MOUSE_LEFT);
            break;
        }
        case WM_MBUTTONDOWN: {
            input_mouse_down((int32_t) MOUSE_MIDDLE);
            break;
        }
        case WM_MBUTTONUP: {
            input_mouse_up((int32_t) MOUSE_MIDDLE);
            break;
        }
        case WM_MOUSEWHEEL: {
            const int32_t scrollDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            input_mouse_scroll(scrollDelta);
            break;
        }
        case WM_XBUTTONDOWN: {
            switch (GET_XBUTTON_WPARAM(wParam)) {
                case XBUTTON1: {
                    input_mouse_down((int32_t) MOUSE_BACK);
                    break;
                }
                case XBUTTON2: {
                    input_mouse_down((int32_t) MOUSE_FORWARD);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case WM_XBUTTONUP: {
            switch (GET_XBUTTON_WPARAM(wParam)) {
                case XBUTTON1: {
                    input_mouse_up((int32_t) MOUSE_BACK);
                    break;
                }
                case XBUTTON2: {
                    input_mouse_up((int32_t) MOUSE_FORWARD);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void update_cursor_state() {
    switch (s_windowInfo.currentCursorState) {
        case CURSOR_HIDDEN_LOCKED_LOCK_MOUSE_POS: {
            SetCursorPos(s_windowInfo.lockedCursorPosition.x, s_windowInfo.lockedCursorPosition.y);
            break;
        }
        default:
            break;
    }
}

static void check_cursor_over_window(MSG *msg) {
    POINT point;
    GetCursorPos(&point);
    const RECT rect = (RECT){
            s_windowInfo.posX,
            s_windowInfo.posY,
            s_windowInfo.posX + s_windowInfo.width,
            s_windowInfo.posY + s_windowInfo.height,
    };
    bool cursorOverWindow = !(point.x <= rect.left || point.x >= rect.right || point.y >= rect.bottom ||
                              point.y <= rect.top);
    if (cursorOverWindow) {
        if (!s_windowInfo.cursorOverWindow) {
            s_windowInfo.cursorOverWindow = true;
            msg->message = WM_MOUSEHOVER;
            msg->hwnd = s_windowInfo.handle;
            msg->lParam = 0xFFFFFFFF;
        }
    } else {
        if (s_windowInfo.cursorOverWindow) {
            s_windowInfo.cursorOverWindow = false;
            msg->message = WM_MOUSELEAVE;
            msg->hwnd = s_windowInfo.handle;
            msg->lParam = 0xFFFFFFFF;
        }
    }
}

static void window_poll() {
    MSG msg = (MSG){};
    while (PeekMessage(&msg, s_windowInfo.handle, 0, 0, PM_REMOVE)) {
        check_cursor_over_window(&msg);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static void window_update_pos_size() {
    vec2i cvarPos = vec2i_cvar_get(g_coreCvars.windowPosition);
    vec2i cvarSize = vec2i_cvar_get(g_coreCvars.windowSize);
    bool samePos = vec2i_equal(cvarPos, (vec2i) { .x = s_windowInfo.posX, .y = s_windowInfo.posY });
    bool sameSize = vec2i_equal(cvarSize, (vec2i) { .x = s_windowInfo.width, .y = s_windowInfo.height });
    if ( !samePos || !sameSize ) {
        SetWindowPos(s_windowInfo.handle, HWND_TOP, cvarPos.x, cvarPos.y, cvarSize.x, cvarSize.y, SWP_NONE);
    }
}
//=============================================================================

//===API=======================================================================
void window_update() {
    window_poll();
    update_cursor_state();
    window_update_pos_size();
}

bool window_is_open() {
    return !s_windowInfo.shouldWindowClose;
}

bool window_is_cursor_over_window() {
    return s_windowInfo.cursorOverWindow;
}

void window_set_cursor_lock_position(const vec2i lockPos) {
    s_windowInfo.lockedCursorPosition = (vec2i){
            .x = s_windowInfo.posX + lockPos.x,
            .y = s_windowInfo.posY + lockPos.y
    };
}

void window_set_cursor(InputCursorState state) {
    const RECT rect = (RECT){
            s_windowInfo.posX,
            s_windowInfo.posY,
            s_windowInfo.posX + s_windowInfo.width,
            s_windowInfo.posY + s_windowInfo.height,
    };
    s_windowInfo.currentCursorState = state;
    switch (s_windowInfo.currentCursorState) {
        case CURSOR_NORMAL: {
            ShowCursor(true);
            ClipCursor(nullptr);
            break;
        }
        case CURSOR_HIDDEN: {
            ShowCursor(false);
            ClipCursor(nullptr);
            break;
        }
        case CURSOR_LOCKED: {
            ShowCursor(true);
            ClipCursor(&rect);
            break;
        }
        case CURSOR_HIDDEN_LOCKED_LOCK_MOUSE_POS:
        case CURSOR_HIDDEN_LOCKED: {
            ShowCursor(false);
            ClipCursor(&rect);
            break;
        }
        default:
            break;
    }
}

void *window_get_handle() {
    return s_windowInfo.handle;
}

void window_set_procedure_callback_func(void *procCallback) {
    //Cursed casting but makes the API very convenient
    g_windowProcCallback_Func = (LRESULT (*)(HWND, UINT, WPARAM, LPARAM)) (procCallback);
}
//=============================================================================

//===INIT_&_SHUTDOWN===========================================================
static wchar_t s_wc_windowTitle[KAH_MAX_WINDOW_TITLE_SIZE];
void window_create(const char* windowTitle, const vec2i windowSize, const vec2i windowPosition) {
    core_assert(strlen(windowTitle) < KAH_MAX_WINDOW_TITLE_SIZE);
    int32_t screenX = windowPosition.x;
    int32_t screenY = windowPosition.y;
    if (windowPosition.x == INT32_MAX || windowPosition.y == INT32_MAX) {
        screenX = GetSystemMetrics(SM_CXSCREEN) / 2 - (windowSize.x / 2);
        screenY = GetSystemMetrics(SM_CYSCREEN) / 2 - (windowSize.y / 2);
    }
    MultiByteToWideChar(CP_UTF8, 0, windowTitle, -1, &s_wc_windowTitle[0], KAH_MAX_WINDOW_TITLE_SIZE);

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    {
        s_windowInfo = (WindowInfo){
            .windowClass = (WNDCLASSEX){
                .cbSize = sizeof(s_windowInfo.windowClass) ,
                .style = CS_HREDRAW | CS_VREDRAW,
                .lpfnWndProc = window_procedure_callback,
                // .cbClsExtra = ,
                // .cbWndExtra = ,
                .hInstance = hInstance,
                .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
                .hCursor = LoadCursor(nullptr, IDC_ARROW),
                //.hbrBackground = ,
                // .lpszMenuName = ,
                .lpszClassName = L"KAH_WINDOW_APPLICATION_NAME",
                // .hIconSm =
            },
            .handle = nullptr,
            .applicationName = L"KAH_WINDOW_APPLICATION_NAME",
            .titleName = s_wc_windowTitle,
            .width = windowSize.x,
            .height = windowSize.y,
            .posX = screenX,
            .posY = screenY,
            .shouldWindowClose = false,
            .currentCursorState = CURSOR_NORMAL,
            .lockedCursorPosition = (vec2i){},
            .virtualCursorPosition = (vec2i){},
            .cursorOverWindow = false
        };

        RegisterClassEx(&s_windowInfo.windowClass);
    }

    s_windowInfo.handle = CreateWindowEx(
            WS_EX_LEFT,
            s_windowInfo.applicationName,
            s_windowInfo.titleName,
            WS_OVERLAPPEDWINDOW,
            s_windowInfo.posX,
            s_windowInfo.posY,
            s_windowInfo.width,
            s_windowInfo.height,
            nullptr,
            nullptr,
            hInstance,
            nullptr
    );

    core_assert_msg(s_windowInfo.handle, "Err: Failed to create window.");
    ShowWindow(s_windowInfo.handle, SW_SHOWDEFAULT);
    UpdateWindow(s_windowInfo.handle);
    s_windowInfo.shouldWindowClose = false;
    {
        RAWINPUTDEVICE rawInputDevice[1];
        rawInputDevice[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
        rawInputDevice[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
        rawInputDevice[0].dwFlags = RIDEV_INPUTSINK;
        rawInputDevice[0].hwndTarget = s_windowInfo.handle;
        const BOOL result = RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));
        core_assert_msg(result == TRUE, "Failed to register raw input devices");
    }
}

void window_cleanup() {
    DestroyWindow(s_windowInfo.handle);

    s_windowInfo = (WindowInfo){};
}
//=============================================================================
#endif
