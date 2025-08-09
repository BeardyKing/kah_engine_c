#include <kah_core/defines.h>

#if CHECK_FEATURE(FEATURE_PLATFORM_WINDOWS)
//===INCLUDES==================================================================
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_surface.h>
#include <kah_core/assert.h>

#include <Windows.h>
//=============================================================================

//===API=======================================================================
void gfx_create_surface(void* windowHandle, const VkInstance* instance, VkSurfaceKHR* outSurface)
{
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};

    core_assert_msg(windowHandle != nullptr, "Err: invalid window handle");
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = (HWND)windowHandle;

    VkResult result = vkCreateWin32SurfaceKHR(*instance, &surfaceInfo, nullptr, outSurface);
    core_assert_msg(result == VK_SUCCESS, "Err: failed to create vulkan surface");
}
// cleanup exists in gfx_backed_vulkan as it doesn't use any windows APIs
//=============================================================================

#endif
