//===INCLUDES==================================================================
#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan.h>

#include <kah_core/assert.h>
#include <kah_core/dynamic_array.h>
#include <kah_core/c_string.h>

#include <stdio.h>
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
static struct GfxBackend{
    uint32_t instanceVersion;

    VmaAllocator allocator;

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkInstance instance;

    DynamicArray supportedInstanceExtensions;
    DynamicArray supportedValidationLayers;
} s_gfx;
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================

Allocator gfx_allocator(){ return allocators()->cstd;}          // TODO: replace with gfx lifetime arena allocator
Allocator gfx_allocator_arena(){ return allocators()->arena;}   // TODO: replace with gfx frame arena

static void gfx_create_data_structures(){
    s_gfx.supportedInstanceExtensions = dynamic_array_create(gfx_allocator(), sizeof(VkExtensionProperties),0);
    s_gfx.supportedValidationLayers = dynamic_array_create(gfx_allocator(), sizeof(VkLayerProperties), 0);
}

static void gfx_cleanup_data_structures(){
    dynamic_array_cleanup(gfx_allocator(), &s_gfx.supportedInstanceExtensions);
    dynamic_array_cleanup(gfx_allocator(), &s_gfx.supportedValidationLayers);
}

static void gfx_volk_create(){
    VkResult volkInitRes = volkInitialize();
    core_assert(volkInitRes == VK_SUCCESS);
    s_gfx.instanceVersion = volkGetInstanceVersion();
}

static void gfx_volk_cleanup(){
    volkFinalize();
}

static void gfx_vma_create(VmaAllocatorCreateInfo allocatorInfo){
    allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    vmaCreateAllocator(
    &(VmaAllocatorCreateInfo){
        .physicalDevice = s_gfx.physicalDevice,
        .device = s_gfx.device,
        .instance = s_gfx.instance,
        .pVulkanFunctions = &(VmaVulkanFunctions){
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr
            }
        },
        &s_gfx.allocator
    );

}

static void gfx_vma_cleanup(){
    vmaDestroyAllocator(s_gfx.allocator);
}

static uint32_t gfx_find_supported_instance_extension_index(const char* extensionName){
    for (uint32_t i = 0; i < s_gfx.supportedInstanceExtensions.count; ++i) {
        const VkExtensionProperties* supportedExtension = dynamic_array_buffer(&s_gfx.supportedInstanceExtensions);
        if ( c_str_equal(supportedExtension[i].extensionName, extensionName)) {
            return i;
        }
    }
    core_assert_msg(false, "failed to find supported extension %s /n", extensionName);
    return false;
}

static void gfx_create_instance(){
    core_assert(s_gfx.supportedInstanceExtensions.current == 0);
    vkEnumerateInstanceExtensionProperties(nullptr, &s_gfx.supportedInstanceExtensions.current, nullptr);
    dynamic_array_resize(gfx_allocator(),&s_gfx.supportedInstanceExtensions, s_gfx.supportedInstanceExtensions.current);
    if (s_gfx.supportedInstanceExtensions.current > 0) {
        vkEnumerateInstanceExtensionProperties(nullptr, &s_gfx.supportedInstanceExtensions.count, s_gfx.supportedInstanceExtensions.info->bufferAddress);
    }

    for (uint32_t i = 0; i < s_gfx.supportedInstanceExtensions.count; ++i) {
        const VkExtensionProperties* supportedExtension = dynamic_array_buffer(&s_gfx.supportedInstanceExtensions);
        printf("Instance extension: %s \n", supportedExtension[i].extensionName);
    }

    char* surfaceName =      dynamic_array_get(&s_gfx.supportedInstanceExtensions,gfx_find_supported_instance_extension_index(VK_KHR_SURFACE_EXTENSION_NAME));
    char* win32SurfaceName = dynamic_array_get(&s_gfx.supportedInstanceExtensions,gfx_find_supported_instance_extension_index(VK_KHR_WIN32_SURFACE_EXTENSION_NAME));
    char* debugUtilName =    dynamic_array_get(&s_gfx.supportedInstanceExtensions,gfx_find_supported_instance_extension_index(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    char* deviceProp2Name =  dynamic_array_get(&s_gfx.supportedInstanceExtensions,gfx_find_supported_instance_extension_index(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME));

    DynamicArray usedInstanceExtensions = dynamic_array_create(gfx_allocator_arena(), sizeof(char**), 4);
    dynamic_array_push(gfx_allocator_arena(), &usedInstanceExtensions, &surfaceName);
    dynamic_array_push(gfx_allocator_arena(), &usedInstanceExtensions, &win32SurfaceName);
    dynamic_array_push(gfx_allocator_arena(), &usedInstanceExtensions, &debugUtilName);
    dynamic_array_push(gfx_allocator_arena(), &usedInstanceExtensions, &deviceProp2Name);

    for (uint32_t i = 0; i < usedInstanceExtensions.current; ++i) {
        char** str = dynamic_array_get(&usedInstanceExtensions, i);
        printf("Used instance extension: %s \n", *str);
    }
}
//=============================================================================

//===API=======================================================================
void gfx_update(){}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_create(void* windowHandle)
{
    gfx_create_data_structures();
    gfx_volk_create();
    gfx_create_instance();

    VmaAllocatorCreateInfo info = {};
    // vma_create(info);
    printf("Vulkan(volk) version %d.%d.%d initialized.\n",VK_VERSION_MAJOR(s_gfx.instanceVersion),VK_VERSION_MINOR(s_gfx.instanceVersion),VK_VERSION_PATCH(s_gfx.instanceVersion));
}

void gfx_cleanup()
{
    gfx_volk_cleanup();
    // vma_cleanup();
    gfx_cleanup_data_structures();
}
//=============================================================================

