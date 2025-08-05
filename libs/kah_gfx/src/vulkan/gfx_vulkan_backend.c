//===INCLUDES==================================================================
#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan.h>

#include <kah_core/assert.h>
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
//=============================================================================

//===API=======================================================================
void gfx_update(){}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_create(void* windowHandle)
{
    VkResult volkInitRes = volkInitialize();
    core_assert(volkInitRes == VK_SUCCESS);
    const uint32_t version = volkGetInstanceVersion();
    VmaAllocatorCreateInfo info = {};
    printf("[gfx] Vulkan(volk) version %d.%d.%d initialized.\n",VK_VERSION_MAJOR(version),VK_VERSION_MINOR(version),VK_VERSION_PATCH(version));
}

void gfx_cleanup()
{
    volkFinalize();
}
//=============================================================================

