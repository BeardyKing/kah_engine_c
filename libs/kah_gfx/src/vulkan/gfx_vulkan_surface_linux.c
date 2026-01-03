#include <kah_core/defines.h>
#if CHECK_FEATURE(FEATURE_PLATFORM_LINUX)

//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_surface.h>
#include <kah_core/assert.h>
//=============================================================================

//===API=======================================================================
struct XLibHandles // must mirror struct in kah_core/src/window_linux.c
{
    Display* display;
    Window window;
};

void gfx_surface_create(void* windowAndDisplayHandle, const VkInstance* instance, VkSurfaceKHR* outSurface){
    core_assert_msg(windowAndDisplayHandle != nullptr, "Err: invalid window & display struct");
    struct XLibHandles* xLibHandles = (struct XLibHandles*)windowAndDisplayHandle;

    VkXlibSurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = xLibHandles->display,
        .window = xLibHandles->window,
    };
    enum VkResult result = vkCreateXlibSurfaceKHR(*instance, &surfaceInfo, nullptr, outSurface);
    core_assert_msg(result == VK_SUCCESS, "Err: failed to create vulkan surface");
}

// cleanup exists in gfx_backed_vulkan as it doesn't use any XLib APIs

//=============================================================================
#endif
