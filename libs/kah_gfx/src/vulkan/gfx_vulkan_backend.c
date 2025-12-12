//===INCLUDES==================================================================
#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/gfx_task_graph.h>
#include <kah_gfx/gfx_logging.h>
#include <kah_gfx/gfx_pool.h>
#include <kah_gfx/gfx_converter.h>
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan_surface.h>
#include <kah_gfx/vulkan/gfx_vulkan_types.h>
#include <kah_gfx/vulkan/gfx_vulkan_utils.h>
#include <kah_gfx/vulkan/gfx_vulkan_imgui.h>
#include <kah_gfx/vulkan/gfx_vulkan_lit.h>
#include <kah_gfx/vulkan/gfx_vulkan_bindless.h>
#include <kah_gfx/vulkan/gfx_vulkan_texture.h>
#include <kah_gfx/vulkan/gfx_vulkan_mesh.h>

#include <kah_core/assert.h>
#include <kah_core/dynamic_array.h>
#include <kah_core/c_string.h>
#include <kah_core/utils.h>
#include <kah_core/bit_array.h>
#include <kah_core/filesystem.h>

#include <kah_math/utils.h>
#include <kah_math/vec2.h>

#include <stdio.h>
//=============================================================================

//===INTERNAL_CONSTANTS/DEFINES================================================
constexpr char KAH_VK_VALIDATION_LAYER_NAME[] = "VK_LAYER_KHRONOS_validation";

constexpr VkDebugUtilsMessageSeverityFlagsEXT KAH_VK_DEBUG_UTILS_MESSAGE_SEVERITY =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

constexpr VkDebugUtilsMessageTypeFlagsEXT KAH_VK_DEBUG_UTILS_MESSAGE_TYPE =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

#define pnext_chain_push_front(mainStruct, newStruct)   \
do {                                                    \
    (newStruct)->pNext = (mainStruct)->pNext;           \
    (mainStruct)->pNext = (void*)(newStruct);           \
} while (0)
//=============================================================================

//===GLOBAL_STRUCTS============================================================
GlobalGfx g_gfx = {};
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
struct GfxSemaphores {
    VkSemaphore presentDone;
    VkSemaphore renderDone;
}typedef GfxSemaphores;

struct GfxSwapChain {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;

    uint32_t imageCount;
    uint32_t currentImageIndex;
    uint32_t lastImageIndex;
    VkImage images[KAH_SWAP_CHAIN_IMAGE_COUNT];
    VkImageView views[KAH_SWAP_CHAIN_IMAGE_COUNT];

    uint32_t width;
    uint32_t height;
}typedef GfxSwapChain;

static struct GfxBackend{
    uint32_t instanceVersion;

    GfxSwapChain swapChain;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[KAH_SWAP_CHAIN_IMAGE_COUNT];
    VkFence graphicsFenceWait[KAH_SWAP_CHAIN_IMAGE_COUNT];
    GfxSemaphores semaphores[KAH_SWAP_CHAIN_IMAGE_COUNT];

    VkCommandBuffer commandBufferImmediate;
    bool commandBufferImmediateInUse;

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

    VkPipelineCache pipelineCache;

    DynamicArray supportedInstanceExtensions;
    DynamicArray supportedValidationLayers;
    DynamicArray supportedphysicalDevices;
    DynamicArray supportedDeviceExtensions;

    uint32_t queueFamilyIndex;

    uint32_t currentGfxFrame;
    bool windowNeedsResize;
} s_gfx = {};

static struct GfxDebug {
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
} s_gfxDebug = {};

static struct GfxUserArguments {
    uint32_t selectedPhysicalDeviceIndex;
    bool vsync;
    VkSampleCountFlagBits msaa;
} s_userArguments = {};

static struct BuiltIns{
    struct{
        GfxTextureHandle black;
        GfxTextureHandle white;
        GfxTextureHandle uvGrid;
    }textures;

    struct{
        GfxMeshHandle quad;
    }meshes;
} s_builtIn;

static struct GfxFeatures{
    VkPhysicalDeviceFeatures2 deviceFeatures;
    VkPhysicalDeviceVulkan11Features features11;
    VkPhysicalDeviceVulkan12Features features12;
    VkPhysicalDeviceVulkan13Features features13;
    VkPhysicalDeviceVulkan14Features features14;

    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchainFeatures;
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicState1Features;
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dynamicState2Features;
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features;
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures;
    VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphoreFeatures;
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2Features;
} s_gfxFeatures = {};

//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
static Allocator gfx_allocator(){ return allocators()->cstd;}          // TODO: replace with gfx lifetime arena allocator
static Allocator gfx_allocator_arena(){ return allocators()->arena;}   // TODO: replace with gfx frame arena

static void gfx_data_structures_create(){
    s_gfx = (struct GfxBackend){};
    s_gfxDebug = (struct GfxDebug){};
    s_userArguments = (struct GfxUserArguments){ //TODO: replace with quake style CVAR system
        .selectedPhysicalDeviceIndex = 0,
        .vsync = true,
        .msaa = VK_SAMPLE_COUNT_1_BIT
    };

    s_gfxFeatures = (struct GfxFeatures){
        .deviceFeatures =           (VkPhysicalDeviceFeatures2){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2} ,
        .features11 =               (VkPhysicalDeviceVulkan11Features){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES } ,
        .features12 =               (VkPhysicalDeviceVulkan12Features){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES } ,
        .features13 =               (VkPhysicalDeviceVulkan13Features){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES } ,
        .features14 =               (VkPhysicalDeviceVulkan14Features){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES } ,
        .swapchainFeatures =        (VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT } ,
        .dynamicState1Features =    (VkPhysicalDeviceExtendedDynamicStateFeaturesEXT){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT } ,
        .dynamicState2Features =    (VkPhysicalDeviceExtendedDynamicState2FeaturesEXT){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT } ,
        .dynamicState3Features =    (VkPhysicalDeviceExtendedDynamicState3FeaturesEXT){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT } ,
        .dynamicRenderingFeatures = (VkPhysicalDeviceDynamicRenderingFeaturesKHR){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR },
        .timelineSemaphoreFeatures = (VkPhysicalDeviceTimelineSemaphoreFeaturesKHR){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR },
        .synchronization2Features = (VkPhysicalDeviceSynchronization2FeaturesKHR){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR },
    };

    s_gfx.supportedInstanceExtensions = dynamic_array_create(gfx_allocator(), sizeof(VkExtensionProperties),0);
    s_gfx.supportedValidationLayers = dynamic_array_create(gfx_allocator(), sizeof(VkLayerProperties), 0);
    s_gfx.supportedphysicalDevices = dynamic_array_create(gfx_allocator(), sizeof(VkPhysicalDevice), 0);
    s_gfx.supportedDeviceExtensions = dynamic_array_create(gfx_allocator(), sizeof(VkExtensionProperties), 0);
}

static void gfx_data_structures_cleanup(){
    dynamic_array_cleanup(gfx_allocator(), &s_gfx.supportedInstanceExtensions);
    dynamic_array_cleanup(gfx_allocator(), &s_gfx.supportedValidationLayers);
    dynamic_array_cleanup(gfx_allocator(), &s_gfx.supportedphysicalDevices);
    dynamic_array_cleanup(gfx_allocator(), &s_gfx.supportedDeviceExtensions);
}

static void gfx_volk_create(){
    VkResult volkInitRes = volkInitialize();
    core_assert(volkInitRes == VK_SUCCESS);
    s_gfx.instanceVersion = volkGetInstanceVersion();
    gfx_log_info("Vulkan(volk) version %d.%d.%d initialized.\n",VK_VERSION_MAJOR(s_gfx.instanceVersion),VK_VERSION_MINOR(s_gfx.instanceVersion),VK_VERSION_PATCH(s_gfx.instanceVersion));
}

static void gfx_volk_cleanup(){
    volkFinalize();
}

static void gfx_vma_create(){
    const VmaAllocatorCreateInfo allocatorInfo = (VmaAllocatorCreateInfo){
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = g_gfx.physicalDevice,
        .device = g_gfx.device ,
        .pAllocationCallbacks = g_gfx.allocationCallbacks,
        .pVulkanFunctions = &(VmaVulkanFunctions){
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
            .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
            .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
            .vkAllocateMemory = vkAllocateMemory,
            .vkFreeMemory = vkFreeMemory,
            .vkMapMemory = vkMapMemory,
            .vkUnmapMemory = vkUnmapMemory,
            .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
            .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
            .vkBindBufferMemory = vkBindBufferMemory,
            .vkBindImageMemory = vkBindImageMemory,
            .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
            .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
            .vkCreateBuffer = vkCreateBuffer,
            .vkDestroyBuffer = vkDestroyBuffer,
            .vkCreateImage = vkCreateImage,
            .vkDestroyImage = vkDestroyImage,
            .vkCmdCopyBuffer = vkCmdCopyBuffer,
            .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
            .vkGetImageMemoryRequirements2KHR = *vkGetImageMemoryRequirements2KHR,
            .vkBindBufferMemory2KHR = vkBindBufferMemory2KHR,
            .vkBindImageMemory2KHR = vkBindImageMemory2KHR,
            .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR,
            .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
            .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
        },
        .instance = g_gfx.instance,
        // .vulkanApiVersion = ,
        // .preferredLargeHeapBlockSize = ,
        // .pDeviceMemoryCallbacks = ,
        // .pHeapSizeLimit = ,
        // .pTypeExternalMemoryHandleTypes =
    };

    const VkResult vmaResult = vmaCreateAllocator( &allocatorInfo, &g_gfx.allocator );
    core_assert_msg(vmaResult == VK_SUCCESS, "err: failed to init VMA");
}

static void gfx_vma_cleanup(){
    vmaDestroyAllocator(g_gfx.allocator);
}

static uint32_t gfx_find_supported_instance_extension_index(const char* extensionName){
    for (uint32_t i = 0; i < s_gfx.supportedInstanceExtensions.count; ++i) {
        const VkExtensionProperties* supportedExtension = dynamic_array_buffer(&s_gfx.supportedInstanceExtensions);
        if ( c_str_equal(supportedExtension[i].extensionName, extensionName)) {
            return i;
        }
    }
    core_assert_msg(false, "failed to find supported extension %s /n", extensionName);
    return UINT32_MAX;
}


[[maybe_unused]] static uint32_t gfx_find_supported_device_extension_index(const char* extensionName){
    for (uint32_t i = 0; i < s_gfx.supportedDeviceExtensions.count; ++i) {
        const VkExtensionProperties* supportedExtension = dynamic_array_buffer(&s_gfx.supportedDeviceExtensions);
        if ( c_str_equal(supportedExtension[i].extensionName, extensionName)) {
            return i;
        }
    }
    core_assert_msg(false, "failed to find supported extension %s /n", extensionName);
    return UINT32_MAX;
}


static const char* gfx_find_supported_device_extension_name(const char* extensionName){
    for (uint32_t i = 0; i < s_gfx.supportedDeviceExtensions.count; ++i) {
        const VkExtensionProperties* supportedExtension = dynamic_array_buffer(&s_gfx.supportedDeviceExtensions);
        if ( c_str_equal(supportedExtension[i].extensionName, extensionName)) {
            return &supportedExtension[i].extensionName[0];
        }
    }
    return nullptr;
}

static uint32_t gfx_find_supported_validation_layer(const char *layerName) {
    for (uint32_t i = 0; i < s_gfx.supportedValidationLayers.count; ++i) {
        VkLayerProperties* supportedLayers = dynamic_array_buffer(&s_gfx.supportedValidationLayers);
        if (c_str_equal(supportedLayers[i].layerName, layerName)) {
            return i;
        }
    }
    core_assert_msg(false, "failed to find supported validation layer %s /n", layerName);
    return UINT32_MAX;
}

static void gfx_instance_create(){
    //===INSTANCE_EXTENSIONS===================================================
    core_assert(s_gfx.supportedInstanceExtensions.current == 0);
    vkEnumerateInstanceExtensionProperties(nullptr, &s_gfx.supportedInstanceExtensions.current, nullptr);
    dynamic_array_resize(gfx_allocator(),&s_gfx.supportedInstanceExtensions, s_gfx.supportedInstanceExtensions.current);

    if (s_gfx.supportedInstanceExtensions.current > 0) {
        vkEnumerateInstanceExtensionProperties(nullptr, &s_gfx.supportedInstanceExtensions.count, s_gfx.supportedInstanceExtensions.info->bufferAddress);
    }

    VkExtensionProperties* supportedExtension = dynamic_array_buffer(&s_gfx.supportedInstanceExtensions);
    for (uint32_t i = 0; i < s_gfx.supportedInstanceExtensions.count; ++i) {
        gfx_log_verbose("Supported instance extension: %s \n", supportedExtension[i].extensionName);
    }

    char* surfaceName      = ((VkExtensionProperties*)dynamic_array_get(&s_gfx.supportedInstanceExtensions,gfx_find_supported_instance_extension_index(VK_KHR_SURFACE_EXTENSION_NAME)))->extensionName;
    char* win32SurfaceName = ((VkExtensionProperties*)dynamic_array_get(&s_gfx.supportedInstanceExtensions,gfx_find_supported_instance_extension_index(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)))->extensionName;
    char* debugUtilName    = ((VkExtensionProperties*)dynamic_array_get(&s_gfx.supportedInstanceExtensions,gfx_find_supported_instance_extension_index(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)))->extensionName;
    char* deviceProp2Name  = ((VkExtensionProperties*)dynamic_array_get(&s_gfx.supportedInstanceExtensions,gfx_find_supported_instance_extension_index(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)))->extensionName;

    DynamicArray usedInstanceExtensions = dynamic_array_create(gfx_allocator_arena(), sizeof(char*), 4);
    dynamic_array_push(gfx_allocator_arena(), &usedInstanceExtensions, &surfaceName);
    dynamic_array_push(gfx_allocator_arena(), &usedInstanceExtensions, &win32SurfaceName);
    dynamic_array_push(gfx_allocator_arena(), &usedInstanceExtensions, &debugUtilName);
    dynamic_array_push(gfx_allocator_arena(), &usedInstanceExtensions, &deviceProp2Name);

    for (uint32_t i = 0; i < usedInstanceExtensions.current; ++i) {
        char* extName = *(char**)dynamic_array_get(&usedInstanceExtensions, i);
        gfx_log_info("Used instance extension: %s\n", extName);
    }
    //=========================================================================

    //===VALIDATION_LAYERS=====================================================
    core_assert(s_gfx.supportedValidationLayers.current == 0);
    vkEnumerateInstanceLayerProperties(&s_gfx.supportedValidationLayers.current, nullptr);
    dynamic_array_resize(gfx_allocator(),&s_gfx.supportedValidationLayers, s_gfx.supportedValidationLayers.current);

    if (s_gfx.supportedValidationLayers.count > 0) {
        VkLayerProperties * supportedValidationLayers = dynamic_array_buffer(&s_gfx.supportedValidationLayers);
        vkEnumerateInstanceLayerProperties(&s_gfx.supportedValidationLayers.count, supportedValidationLayers);
    }

    VkLayerProperties* supportedValidationLayers = dynamic_array_buffer(&s_gfx.supportedValidationLayers);
    for (uint32_t i = 0; i < s_gfx.supportedValidationLayers.count; ++i) {
        gfx_log_verbose("Layer: %s - Desc: %s\n", supportedValidationLayers[i].layerName, supportedValidationLayers->description);
    }

    char* validationLayerName = ((VkLayerProperties*)dynamic_array_get(&s_gfx.supportedValidationLayers,gfx_find_supported_validation_layer(KAH_VK_VALIDATION_LAYER_NAME)))->layerName;

    DynamicArray usedValidationLayers = dynamic_array_create(gfx_allocator_arena(), sizeof(char*), 1);
    dynamic_array_push(gfx_allocator_arena(), &usedValidationLayers, &validationLayerName);

    for (uint32_t i = 0; i < usedValidationLayers.current; ++i) {
        char* str = *(char**)dynamic_array_get(&usedValidationLayers, i);
        gfx_log_info("Used validation layers: %s \n", str);
    }
    //=========================================================================

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "KAH_ENGINE",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "KAH_ENGINE",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_MAKE_VERSION(1,2,0), //vulkan 1.2 support required
    };

    VkInstanceCreateInfo instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = usedValidationLayers.count,
        .ppEnabledLayerNames = dynamic_array_buffer(&usedValidationLayers),
        .enabledExtensionCount = usedInstanceExtensions.count,
        .ppEnabledExtensionNames = dynamic_array_buffer(&usedInstanceExtensions),
    };

    const VkResult result = vkCreateInstance(&instanceInfo, g_gfx.allocationCallbacks, &g_gfx.instance);
    core_assert_msg(result == VK_SUCCESS, "err: failed to create vulkan instance");

    volkLoadInstance(g_gfx.instance); // load instnace function pointers
    core_assert(vkDestroyInstance != VK_NULL_HANDLE); // validate callbacks are setup
    //=========================================================================
}

static void gfx_instance_cleanup(){
    core_assert_msg(g_gfx.instance != VK_NULL_HANDLE, "err: VkInstance has already been destroyed");
    vkDestroyInstance(g_gfx.instance, g_gfx.allocationCallbacks);
}

static VkBool32 VKAPI_PTR validation_message_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageWarningLevel,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
        void */*userData*/) {
    core_assert(callbackData != nullptr);
    switch (messageWarningLevel) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            gfx_log_error("[ERROR]: \ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            gfx_log_warning("[WARNING]:\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            gfx_log_info("[INFO]: \ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            gfx_log_verbose("[VERBOSE]: \ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        default: {
            core_assert(callbackData && callbackData->pMessageIdName && callbackData->pMessage);
        }
    }
    return VK_FALSE;
}

static void gfx_debug_callbacks_create() {
    const VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = KAH_VK_DEBUG_UTILS_MESSAGE_SEVERITY,
        .messageType = KAH_VK_DEBUG_UTILS_MESSAGE_TYPE,
        .pfnUserCallback = validation_message_callback,
    };
    vkCreateDebugUtilsMessengerEXT(g_gfx.instance, &messengerCreateInfo, g_gfx.allocationCallbacks, &s_gfxDebug.debugUtilsMessenger);
}

static void gfx_debug_callbacks_cleanup() {
    core_assert_msg(s_gfxDebug.debugUtilsMessenger != VK_NULL_HANDLE, "err: debug utils messenger has already been destroyed");
    vkDestroyDebugUtilsMessengerEXT(g_gfx.instance, s_gfxDebug.debugUtilsMessenger, g_gfx.allocationCallbacks);
    s_gfxDebug.debugUtilsMessenger = VK_NULL_HANDLE;
}

static const char* device_type_to_string(VkPhysicalDeviceType type) {
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
    default: return "Unknown";
    }
    return "Unknown";
}

static void debug_print_supported_physical_devices_info(){
    const VkPhysicalDevice* physicalDevices = dynamic_array_buffer(&s_gfx.supportedphysicalDevices);

    gfx_log_info("=== PHYSICAL DEVICE INFO ======\n\n");
    gfx_log_info("Found %u Vulkan device(s):\n", s_gfx.supportedphysicalDevices.count);
    for (uint32_t i = 0; i < s_gfx.supportedphysicalDevices.count; ++i) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memProps);

        VkDeviceSize totalDeviceLocalMem = 0;
        for (uint32_t j = 0; j < memProps.memoryHeapCount; ++j) {
            if (memProps.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                totalDeviceLocalMem += memProps.memoryHeaps[j].size;
            }
        }

        gfx_log_info("Device %u: %s\n", i, props.deviceName);
        gfx_log_info("\tType   : %s\n", device_type_to_string(props.deviceType));
        gfx_log_info("\tMemory : %.2f MiB of device-local memory\n", (float)totalDeviceLocalMem / (float)KAH_MiB);
        gfx_log_info("\n");
    }
    gfx_log_info("===============================\n");
}

static void debug_print_selected_physical_device_info(){
    VkDeviceSize totalDeviceLocalMem = 0;
    for (uint32_t j = 0; j < s_gfx.deviceMemoryProperties.memoryHeapCount; ++j) {
        if (s_gfx.deviceMemoryProperties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            totalDeviceLocalMem += s_gfx.deviceMemoryProperties.memoryHeaps[j].size;
        }
    }

    const uint32_t apiVersionMajor = VK_API_VERSION_MAJOR(s_gfx.deviceProperties.apiVersion);
    const uint32_t apiVersionMinor = VK_API_VERSION_MINOR(s_gfx.deviceProperties.apiVersion);
    const uint32_t apiVersionPatch = VK_API_VERSION_PATCH(s_gfx.deviceProperties.apiVersion);

    const uint32_t driverVersionMajor = VK_API_VERSION_MAJOR(s_gfx.deviceProperties.driverVersion);
    const uint32_t driverVersionMinor = VK_API_VERSION_MINOR(s_gfx.deviceProperties.driverVersion);
    const uint32_t driverVersionPatch = VK_API_VERSION_PATCH(s_gfx.deviceProperties.driverVersion);

    const char *deviceType = device_type_to_string(s_gfx.deviceProperties.deviceType);

    gfx_log_info("Selected physical device\n");
    gfx_log_info("Name:\t\t%s \nType:\t\t%s \nMemory:\t\t%.2f MiB \nHeap Count:\t%u \nVersion:\t%u.%u.%u \nDriver: \t%u.%u.%u\n",
             s_gfx.deviceProperties.deviceName,
             deviceType,
             (float)totalDeviceLocalMem / (float)KAH_MiB,
             s_gfx.deviceMemoryProperties.memoryHeapCount,
             apiVersionMajor,
             apiVersionMinor,
             apiVersionPatch,
             driverVersionMajor,
             driverVersionMinor,
             driverVersionPatch
    );
}

static VkSampleCountFlagBits set_target_sample_count(const VkPhysicalDeviceProperties deviceProperties, VkSampleCountFlagBits target) {
    VkSampleCountFlags supportedSampleCount = min_u32(deviceProperties.limits.framebufferColorSampleCounts, deviceProperties.limits.framebufferDepthSampleCounts);
    return (supportedSampleCount & target) ? target : VK_SAMPLE_COUNT_1_BIT;
}

static void gfx_physical_device_create(){
    core_assert(s_gfx.supportedphysicalDevices.current == 0);
    vkEnumeratePhysicalDevices(g_gfx.instance, &s_gfx.supportedphysicalDevices.current, nullptr);
    core_assert_msg(s_gfx.supportedphysicalDevices.current != 0, "err: did not find any vulkan compatible physical devices");
    dynamic_array_resize(gfx_allocator(),&s_gfx.supportedphysicalDevices, s_gfx.supportedphysicalDevices.current);

    VkPhysicalDevice* physicalDevices = dynamic_array_buffer(&s_gfx.supportedphysicalDevices);
    vkEnumeratePhysicalDevices(g_gfx.instance, &s_gfx.supportedphysicalDevices.count, physicalDevices);

    debug_print_supported_physical_devices_info();

    // TODO:GFX: Add fallback support for `best` GPU based on intended workload, if no argument is provided we fallback to device [0]
    uint32_t selectedDevice = s_userArguments.selectedPhysicalDeviceIndex;
    core_assert_msg(selectedDevice < s_gfx.supportedphysicalDevices.count, "err: selecting physical vulkan device that is out of range");
    g_gfx.physicalDevice = physicalDevices[selectedDevice];

    vkGetPhysicalDeviceProperties(g_gfx.physicalDevice, &s_gfx.deviceProperties);
    g_gfx.sampleCount = set_target_sample_count(s_gfx.deviceProperties, s_userArguments.msaa);

    vkGetPhysicalDeviceFeatures(g_gfx.physicalDevice, &s_gfx.deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(g_gfx.physicalDevice, &s_gfx.deviceMemoryProperties);

    debug_print_selected_physical_device_info();
}

static void gfx_surface_cleanup() {
    // gfx_create_surface(...) exists in gfx_vulkan_surface.h / gfx_vulkan_surface_windows.cpp
    core_assert_msg(s_gfx.swapChain.surface != VK_NULL_HANDLE, "err: VkSurface has already been destroyed");
    vkDestroySurfaceKHR(g_gfx.instance, s_gfx.swapChain.surface, g_gfx.allocationCallbacks);
    s_gfx.swapChain.surface = VK_NULL_HANDLE;
}

static void gfx_physical_device_queues_create(){
    vkEnumerateDeviceExtensionProperties(g_gfx.physicalDevice, nullptr, &s_gfx.supportedDeviceExtensions.current, nullptr);
    dynamic_array_resize(gfx_allocator(), &s_gfx.supportedDeviceExtensions, s_gfx.supportedDeviceExtensions.current);
    VkExtensionProperties* supportedDeviceExtensions = dynamic_array_buffer(&s_gfx.supportedDeviceExtensions);

    if (s_gfx.supportedDeviceExtensions.count > 0) {
        vkEnumerateDeviceExtensionProperties(g_gfx.physicalDevice, nullptr, &s_gfx.supportedDeviceExtensions.count, supportedDeviceExtensions);
    }

    for (uint32_t i = 0; i < s_gfx.supportedDeviceExtensions.count; ++i) {
        gfx_log_verbose("Supported device extensions: %s \n", supportedDeviceExtensions[i].extensionName);
    }

    DynamicArray selectedQueueFamilies = dynamic_array_create(gfx_allocator_arena(), sizeof(VkQueueFamilyProperties),0);
    vkGetPhysicalDeviceQueueFamilyProperties(g_gfx.physicalDevice, &selectedQueueFamilies.current, nullptr);
    dynamic_array_resize(gfx_allocator_arena(), &selectedQueueFamilies,selectedQueueFamilies.current);

    VkQueueFamilyProperties* queueFamilies = dynamic_array_buffer(&selectedQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(g_gfx.physicalDevice, &selectedQueueFamilies.count, queueFamilies);

    struct QueueInfo {
        uint32_t targetFlags;
        uint32_t queueIndex;
        uint32_t currentFlags;
        bool supportsPresent;
    }typedef QueueInfo;

    QueueInfo graphicsQueueInfo = (QueueInfo){ //TODO: find queue that has all the features, was failing submit validation
        .targetFlags = VK_QUEUE_GRAPHICS_BIT,// | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
        .queueIndex = UINT32_MAX,
        .currentFlags = UINT32_MAX,
        .supportsPresent = false
    };

    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < selectedQueueFamilies.count; ++queueFamilyIndex){
        VkBool32 supportsPresent = VK_FALSE;
        VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(g_gfx.physicalDevice,queueFamilyIndex,s_gfx.swapChain.surface,&supportsPresent);

        const uint32_t currentQueueFlags = queueFamilies[queueFamilyIndex].queueFlags;

        if ( presentResult >= 0 && supportsPresent == VK_TRUE){
            if (currentQueueFlags & graphicsQueueInfo.targetFlags) {
                if (u32_count_set_bits(currentQueueFlags) < u32_count_set_bits(graphicsQueueInfo.currentFlags)) {
                    graphicsQueueInfo.currentFlags = currentQueueFlags;
                    graphicsQueueInfo.queueIndex = queueFamilyIndex;
                    graphicsQueueInfo.supportsPresent = supportsPresent;
                }
            }
        }
    }
    gfx_log_verbose("Graphics queue index: %u \n", graphicsQueueInfo.queueIndex);
    core_assert_msg(graphicsQueueInfo.queueIndex != UINT32_MAX, "err: did not find valid graphics queue");

    constexpr float graphicsQueuePriority = 1.0f;
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = graphicsQueueInfo.queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &graphicsQueuePriority
    };

    DynamicArray usedDeviceExtensions = dynamic_array_create(gfx_allocator_arena(), sizeof(const char*), 8);

    const char* swapChainName        = gfx_find_supported_device_extension_name( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    const char* dynamicRenderingName = gfx_find_supported_device_extension_name( VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME );
    const char* pushDescriptor       = gfx_find_supported_device_extension_name( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME );
    const char* extDynamicState1     = gfx_find_supported_device_extension_name( VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME );
    const char* extDynamicState2     = gfx_find_supported_device_extension_name( VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME );
    const char* extDynamicState3     = gfx_find_supported_device_extension_name( VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME );
    const char* swapchainMaintence1  = gfx_find_supported_device_extension_name( VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME );
    const char* timelineSemaphore    = gfx_find_supported_device_extension_name( VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME );
    const char* synchronization2    = gfx_find_supported_device_extension_name( VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME );

    s_gfxFeatures.deviceFeatures.pNext = &s_gfxFeatures.features11;
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,2,0)){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.features12);
    }
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,3,0)){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.features13);
    }
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,4,0)){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.features14);
    }

    if(swapChainName){
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &swapChainName);
    }
    if(pushDescriptor){
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &pushDescriptor);
    }
    if(swapchainMaintence1){
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &swapchainMaintence1);
    }
    if(extDynamicState1){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicState1Features);
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &extDynamicState1);
    }
    if(extDynamicState2){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicState2Features);
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &extDynamicState2);
    }
    if(extDynamicState3){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicState3Features);
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &extDynamicState3);
    }
    if(s_gfx.deviceProperties.apiVersion < VK_MAKE_VERSION(1,3,0)){
        if( dynamicRenderingName){
            pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicRenderingFeatures);
            dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &dynamicRenderingName);
        }
        if( synchronization2){
            pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.synchronization2Features);
            dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &synchronization2);
        }
    }
    if(s_gfx.deviceProperties.apiVersion < VK_MAKE_VERSION(1,2,0) ){
        if(timelineSemaphore){
            pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.timelineSemaphoreFeatures);
            dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &timelineSemaphore);
        }
    }

    vkGetPhysicalDeviceFeatures2(g_gfx.physicalDevice, &s_gfxFeatures.deviceFeatures);
    core_assert_msg(s_gfxFeatures.features12.descriptorIndexing, "err: Descriptor indexing is required");
    core_assert_msg(s_gfxFeatures.features12.bufferDeviceAddress, "err: Buffer device address is required");
    core_assert_msg(s_gfxFeatures.features12.timelineSemaphore || s_gfxFeatures.timelineSemaphoreFeatures.timelineSemaphore, "err: Timeline semaphores is required");
    core_assert_msg(s_gfxFeatures.features13.dynamicRendering || s_gfxFeatures.dynamicRenderingFeatures.dynamicRendering, "err: Dynamic rendering is required");
    core_assert_msg(s_gfxFeatures.features13.synchronization2 || s_gfxFeatures.synchronization2Features.synchronization2, "err: Synchronization 2 is required");

    s_gfxFeatures.deviceFeatures.features.robustBufferAccess = false; //Fixes VUID-VkDeviceCreateInfo-robustBufferAccess-10247 when used with descriptorIndexing.

    const char** extName = dynamic_array_buffer(&usedDeviceExtensions);
    for (uint32_t i = 0; i < usedDeviceExtensions.current; ++i){
        gfx_log_info("Selected device extensions: %s \n", extName[i]);
    }

    VkDeviceCreateInfo deviceCreateInfo = (VkDeviceCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &s_gfxFeatures.deviceFeatures,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &graphicsQueueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = usedDeviceExtensions.current,
        .ppEnabledExtensionNames = dynamic_array_buffer(&usedDeviceExtensions)
    };

    vkCreateDevice(g_gfx.physicalDevice, &deviceCreateInfo, g_gfx.allocationCallbacks, &g_gfx.device);
    core_assert_msg(g_gfx.device, "err: Failed to create vkDevice");

    vkGetDeviceQueue(g_gfx.device, graphicsQueueInfo.queueIndex, 0, &g_gfx.queue);
    core_assert_msg(g_gfx.queue, "err: Failed to create graphics queue");

    s_gfx.queueFamilyIndex =  graphicsQueueInfo.queueIndex;
}

static void gfx_physical_device_queues_cleanup(){
    vkDestroyDevice(g_gfx.device, g_gfx.allocationCallbacks);
    g_gfx.device = VK_NULL_HANDLE;
    g_gfx.queue = VK_NULL_HANDLE;
}

static void gfx_command_pool_create(){
    VkCommandPoolCreateInfo commandPoolInfo = (VkCommandPoolCreateInfo){
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex =  s_gfx.queueFamilyIndex,
    };
    const VkResult result = vkCreateCommandPool(g_gfx.device, &commandPoolInfo, g_gfx.allocationCallbacks, &s_gfx.commandPool);
    core_assert_msg(result == VK_SUCCESS, "err: failed to create graphics command pool");
}

static void gfx_command_pool_cleanup(){
    vkDestroyCommandPool(g_gfx.device, s_gfx.commandPool, g_gfx.allocationCallbacks);
    s_gfx.commandPool = VK_NULL_HANDLE;
}

static void gfx_semaphores_create(){
    for (uint32_t i = 0; i < KAH_SWAP_CHAIN_IMAGE_COUNT; ++i) {
        VkSemaphoreCreateInfo semaphoreInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        const VkResult presentRes = vkCreateSemaphore(g_gfx.device, &semaphoreInfo, g_gfx.allocationCallbacks, &s_gfx.semaphores[i].presentDone);
        const VkResult renderRes = vkCreateSemaphore(g_gfx.device, &semaphoreInfo, g_gfx.allocationCallbacks, &s_gfx.semaphores[i].renderDone);
        core_assert_msg(presentRes == VK_SUCCESS, "err: failed to create present semaphore");
        core_assert_msg(renderRes  == VK_SUCCESS, "err: failed to create render semaphore");
    }
}


static void gfx_semaphores_cleanup(){
    for (uint32_t i = 0; i < KAH_SWAP_CHAIN_IMAGE_COUNT; ++i) {
        vkDestroySemaphore(g_gfx.device, s_gfx.semaphores[i].presentDone, g_gfx.allocationCallbacks);
        vkDestroySemaphore(g_gfx.device, s_gfx.semaphores[i].renderDone, g_gfx.allocationCallbacks);
    }
}


static VkPresentModeKHR select_present_mode() {
    DynamicArray devicePresentModes = dynamic_array_create(gfx_allocator_arena(), sizeof(VkPresentModeKHR), 0);
    const VkResult presentRes = vkGetPhysicalDeviceSurfacePresentModesKHR(g_gfx.physicalDevice, s_gfx.swapChain.surface, &devicePresentModes.current, nullptr);
    core_assert_msg(presentRes == VK_SUCCESS, "err: failed to get physical device surface present modes");
    core_assert(devicePresentModes.current > 0);
    dynamic_array_resize(gfx_allocator_arena(), &devicePresentModes, devicePresentModes.current);

    VkPresentModeKHR* presentModes = dynamic_array_buffer(&devicePresentModes);
    const VkResult populateRes = vkGetPhysicalDeviceSurfacePresentModesKHR(g_gfx.physicalDevice, s_gfx.swapChain.surface, &devicePresentModes.count, presentModes);
    core_assert_msg(populateRes == VK_SUCCESS, "err: failed to populate present modes array");

    VkPresentModeKHR selectedPresentMode = {VK_PRESENT_MODE_FIFO_KHR};
    VkPresentModeKHR preferredMode = s_userArguments.vsync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (uint32_t i = 0; i < devicePresentModes.count; ++i) {
        if (presentModes[i] == preferredMode) {
            selectedPresentMode = presentModes[i];
            break;
        }
    }
    return selectedPresentMode;
}

VkCompositeAlphaFlagBitsKHR select_composite_alpha_format(const VkSurfaceCapabilitiesKHR* surfaceCapabilities) {
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    constexpr uint32_t KAH_COMPOSITE_ALPHA_FLAGS_COUNT = 4;
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[KAH_COMPOSITE_ALPHA_FLAGS_COUNT] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };

    for (uint32_t i = 0; i < KAH_COMPOSITE_ALPHA_FLAGS_COUNT; ++i) {
        if (surfaceCapabilities->supportedCompositeAlpha & compositeAlphaFlags[i]) {
            compositeAlpha = compositeAlphaFlags[i];
            break;
        };
    }
    return compositeAlpha;
}

static void gfx_swap_chain_create(){
    const VkSwapchainKHR oldSwapChain = s_gfx.swapChain.swapChain;

    VkSurfaceCapabilitiesKHR surfaceCapabilities = {0};
    const VkResult surfRes = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_gfx.physicalDevice, s_gfx.swapChain.surface, &surfaceCapabilities);
    core_assert_msg(surfRes == VK_SUCCESS, "err: failed to get physical device surface capabilities");

    VkExtent2D swapChainExtent = {};
    if (surfaceCapabilities.currentExtent.width == UINT32_MAX) {
        swapChainExtent.width = s_gfx.swapChain.width;
        swapChainExtent.height = s_gfx.swapChain.height;
    } else {
        swapChainExtent = surfaceCapabilities.currentExtent;
        s_gfx.swapChain.width = surfaceCapabilities.currentExtent.width;
        s_gfx.swapChain.height = surfaceCapabilities.currentExtent.height;
    }


    const VkPresentModeKHR swapChainPresentMode = select_present_mode();
    const VkSurfaceFormatKHR surfaceFormat = gfx_vulkan_utils_select_surface_format();
    const VkCompositeAlphaFlagBitsKHR compositeAlphaFormat = select_composite_alpha_format(&surfaceCapabilities);
    core_assert(surfaceCapabilities.maxImageCount >= KAH_SWAP_CHAIN_IMAGE_COUNT );
    core_assert(surfaceCapabilities.minImageCount <= KAH_SWAP_CHAIN_IMAGE_COUNT );

    s_gfx.swapChain.imageCount = KAH_SWAP_CHAIN_IMAGE_COUNT; // consider making swapchain image count dynamic i.e. double to tripple

    VkSurfaceTransformFlagBitsKHR surfaceTransform = {};
    if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        surfaceTransform = surfaceCapabilities.currentTransform;
    }

    VkImageUsageFlags swapchainImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        swapchainImageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        swapchainImageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    VkSwapchainCreateInfoKHR swapChainInfo = (VkSwapchainCreateInfoKHR){
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = s_gfx.swapChain.surface,
        .minImageCount = s_gfx.swapChain.imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = swapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = swapchainImageUsage,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = surfaceTransform,
        .compositeAlpha = compositeAlphaFormat,
        .presentMode = swapChainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapChain
    };

    const VkResult swapChainRes = vkCreateSwapchainKHR(g_gfx.device, &swapChainInfo, g_gfx.allocationCallbacks, &s_gfx.swapChain.swapChain);
    core_assert_msg(swapChainRes == VK_SUCCESS, "err: failed to create swap chain");

    //cleanup vulkan resources
    if (oldSwapChain != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; i++) {
            vkDestroyImageView(g_gfx.device, s_gfx.swapChain.views[i], g_gfx.allocationCallbacks);
        }
        vkDestroySwapchainKHR(g_gfx.device, oldSwapChain, g_gfx.allocationCallbacks);
    }

    vkGetSwapchainImagesKHR(g_gfx.device, s_gfx.swapChain.swapChain, &s_gfx.swapChain.imageCount, nullptr);
    VkResult swapChainImageResult = vkGetSwapchainImagesKHR(g_gfx.device,  s_gfx.swapChain.swapChain, &s_gfx.swapChain.imageCount, &s_gfx.swapChain.images[0] );
    core_assert_msg(swapChainImageResult == VK_SUCCESS, "err: failed to re-create swap chain images");

    for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; i++) {
        VkImageViewCreateInfo swapChainImageViewInfo = (VkImageViewCreateInfo){
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr ,
            .flags = 0,
            .image = nullptr,
            .viewType = VK_IMAGE_VIEW_TYPE_2D ,
            .format = surfaceFormat.format,
            .components = (VkComponentMapping){
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
            .subresourceRange = (VkImageSubresourceRange){
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0 ,
                .layerCount = 1
            }
        };

        swapChainImageViewInfo.image = s_gfx.swapChain.images[i];

        const VkResult imageViewResult = vkCreateImageView(g_gfx.device, &swapChainImageViewInfo, g_gfx.allocationCallbacks, &s_gfx.swapChain.views[i]);
        core_assert_msg(imageViewResult == VK_SUCCESS, "err: Failed to create image view %u", i);
    }
}

static void gfx_swap_chain_cleanup(){
    for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; i++) {
        vkDestroyImageView(g_gfx.device, s_gfx.swapChain.views[i], g_gfx.allocationCallbacks);
    }

    core_assert_msg(s_gfx.swapChain.surface != VK_NULL_HANDLE,"err: swapchain surface has already been destroyed");
    vkDestroySwapchainKHR(g_gfx.device, s_gfx.swapChain.swapChain, g_gfx.allocationCallbacks);
    s_gfx.swapChain.swapChain = VK_NULL_HANDLE;
}

static void gfx_command_buffers_create(){
    {
        const VkCommandBufferAllocateInfo gfxCommandBufferInfo = (VkCommandBufferAllocateInfo){
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = s_gfx.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = KAH_SWAP_CHAIN_IMAGE_COUNT,
        };

        VkResult cmdBuffResult = vkAllocateCommandBuffers(g_gfx.device, &gfxCommandBufferInfo, s_gfx.commandBuffers);
        core_assert_msg(cmdBuffResult == VK_SUCCESS, "err: Failed to create graphics command buffer");
    }
    {
        VkCommandBufferAllocateInfo immediateCommandBufferInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = s_gfx.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        core_assert(s_gfx.commandBufferImmediate == VK_NULL_HANDLE);
        VkResult cmdImmediateBufferResult = vkAllocateCommandBuffers(g_gfx.device, &immediateCommandBufferInfo,&s_gfx.commandBufferImmediate);
        core_assert_msg(cmdImmediateBufferResult == VK_SUCCESS, "err: failed to create immediate command buffer");
    }
}

static void gfx_command_buffers_cleanup(){
    vkFreeCommandBuffers(g_gfx.device, s_gfx.commandPool, KAH_SWAP_CHAIN_IMAGE_COUNT, s_gfx.commandBuffers);
    for (uint32_t i = 0; i < KAH_SWAP_CHAIN_IMAGE_COUNT; ++i) {
        s_gfx.commandBuffers[i] = VK_NULL_HANDLE;
    }
}


static void gfx_fences_create() {
    const VkFenceCreateInfo fenceCreateInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; ++i) {
        const VkResult fenceRes = vkCreateFence(g_gfx.device, &fenceCreateInfo, g_gfx.allocationCallbacks, &s_gfx.graphicsFenceWait[i]);
        core_assert_msg(fenceRes == VK_SUCCESS, "err: Failed to create graphics fence [%u]", i);
    }
}

static void gfx_fences_cleanup() {
    for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; ++i) {
        vkDestroyFence(g_gfx.device, s_gfx.graphicsFenceWait[i], g_gfx.allocationCallbacks);
        s_gfx.graphicsFenceWait[i] = VK_NULL_HANDLE;
    }
}

static void gfx_pipeline_cache_create(){
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    const VkResult cacheRes = vkCreatePipelineCache(g_gfx.device, &pipelineCacheCreateInfo, g_gfx.allocationCallbacks, &s_gfx.pipelineCache);
    core_assert_msg(cacheRes == VK_SUCCESS, "err: Failed to create pipeline cache");
}

static void gfx_mesh_builtin_create(){
    s_builtIn.meshes.quad = gfx_mesh_build_quad();
}

static void gfx_mesh_builtin_cleanup(){
    gfx_mesh_cleanup(s_builtIn.meshes.quad);
}

static void gfx_texture_builtin_create(){
    //TODO: Add wrapper bindless function to GfxTexture.
    s_builtIn.textures.black = gfx_texture_load_from_file("assets/textures/black.dds");
    s_builtIn.textures.white = gfx_texture_load_from_file("assets/textures/white.dds");
    s_builtIn.textures.uvGrid = gfx_texture_load_from_file("assets/textures/UV_Grid/UV_Grid_test.dds");

    GfxTexture* blackTexture = gfx_pool_get_gfx_texture(s_builtIn.textures.black);
    GfxTexture* whiteTexture = gfx_pool_get_gfx_texture(s_builtIn.textures.white);
    GfxTexture* uvGridTexture = gfx_pool_get_gfx_texture(s_builtIn.textures.uvGrid);

    blackTexture->bindlessIndex = KAH_BINDLESS_TEXTURE_BLACK;
    whiteTexture->bindlessIndex = KAH_BINDLESS_TEXTURE_WHITE;
    uvGridTexture->bindlessIndex = KAH_BINDLESS_TEXTURE_UV;

    gfx_bindless_set_image(blackTexture->bindlessIndex, blackTexture->imageView);
    gfx_bindless_set_image(whiteTexture->bindlessIndex, whiteTexture->imageView);
    gfx_bindless_set_image(uvGridTexture->bindlessIndex, uvGridTexture->imageView);
}

static void gfx_texture_builtin_cleanup(){
    gfx_texture_cleanup(s_builtIn.textures.black);
    gfx_texture_cleanup(s_builtIn.textures.white);
    gfx_texture_cleanup(s_builtIn.textures.uvGrid);
}

static void gfx_pipeline_cache_cleanup(){
    vkDestroyPipelineCache(g_gfx.device, s_gfx.pipelineCache, g_gfx.allocationCallbacks);
}

uint32_t gfx_swap_chain_index() {
    return (s_gfx.swapChain.currentImageIndex);
}

uint32_t gfx_last_swap_chain_index() {
    return (s_gfx.swapChain.lastImageIndex);
}

uint32_t gfx_buffer_index() {
    return (s_gfx.currentGfxFrame % KAH_BUFFER_COUNT);
}

void gfx_flush() {
    if (g_gfx.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(g_gfx.device);
    }
}

static VkResult gfx_acquire_next_swap_chain_image() {
    return vkAcquireNextImageKHR(
            g_gfx.device,
            s_gfx.swapChain.swapChain,
            UINT64_MAX,
            s_gfx.semaphores[gfx_last_swap_chain_index()].presentDone,
            VK_NULL_HANDLE,
            &s_gfx.swapChain.currentImageIndex
    );
}


static bool query_has_valid_extent_size() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_gfx.physicalDevice, s_gfx.swapChain.surface, &surfaceCapabilities);
    s_gfx.swapChain.width = surfaceCapabilities.currentExtent.width;
    s_gfx.swapChain.height = surfaceCapabilities.currentExtent.height;
    return s_gfx.swapChain.width != 0 || s_gfx.swapChain.height != 0;
}


static VkResult gfx_present() {
    VkPresentInfoKHR presentInfo = (VkPresentInfoKHR){
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .swapchainCount = 1,
        .pSwapchains = &s_gfx.swapChain.swapChain,
        .pImageIndices = &s_gfx.swapChain.currentImageIndex,
    };

    if (s_gfx.semaphores[gfx_swap_chain_index()].renderDone != VK_NULL_HANDLE) {
        presentInfo.pWaitSemaphores = &s_gfx.semaphores[gfx_swap_chain_index()].renderDone;
        presentInfo.waitSemaphoreCount = 1;
    }

    return vkQueuePresentKHR(g_gfx.queue, &presentInfo);
}

static void gfx_begin_command_recording(VkCommandBuffer cmdBuffer) {
    const VkCommandBufferBeginInfo cmdBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    const VkResult result = vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);
    core_assert_msg(result == VK_SUCCESS, "err: Vulkan failed to begin command buffer recording");
}

static void gfx_end_command_recording(const VkCommandBuffer cmdBuffer) {
    vkEndCommandBuffer(cmdBuffer);
}

static bool gfx_window_resize() {
    gfx_flush();

    gfx_task_graph_cleanup(false);
    gfx_swap_chain_cleanup();

    gfx_swap_chain_create();
    gfx_task_graph_create(false);
    s_gfx.windowNeedsResize = false;
    return true;
}

VkCommandBuffer gfx_command_buffer_start_immediate_recording() {
    core_assert(!s_gfx.commandBufferImmediateInUse);
    s_gfx.commandBufferImmediateInUse = true;
    const VkCommandBufferBeginInfo cmdBufBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(s_gfx.commandBufferImmediate, &cmdBufBeginInfo);
    return s_gfx.commandBufferImmediate;
}

void gfx_command_buffer_end_immediate_recording(VkCommandBuffer cmdBuffer) {
    core_assert(s_gfx.commandBufferImmediateInUse);
    vkEndCommandBuffer(cmdBuffer);

    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdBuffer,
    };

    vkQueueSubmit(g_gfx.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_gfx.queue);// if this is run more than once per frame we should batch
    s_gfx.commandBufferImmediateInUse = false;
}

void gfx_command_buffer_begin_rendering(VkCommandBuffer cmdBuffer, const VkRenderingInfoKHR *renderingInfo) {
    //TODO: Resolve this to KAH engine specific FP
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,3,0)){
        vkCmdBeginRendering(cmdBuffer, renderingInfo);
    }
    vkCmdBeginRenderingKHR(cmdBuffer, renderingInfo);
}

void gfx_command_buffer_end_rendering(VkCommandBuffer cmdBuffer) {
    //TODO: Resolve this to KAH engine specific FP
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,3,0)){
        vkCmdEndRendering(cmdBuffer);
    }
    vkCmdEndRenderingKHR(cmdBuffer);
}

static VkResult gfx_queue_submit_2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence){
    //TODO: Resolve this to KAH engine specific FP
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,3,0)){
        return vkQueueSubmit2(queue, submitCount, pSubmits, fence);
    }
    return vkQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
}

void gfx_command_buffer_insert_memory_barrier(
        VkCommandBuffer cmdBuffer,
        const VkImage *image,
        const VkAccessFlags srcAccessMask,
        const VkAccessFlags dstAccessMask,
        const VkImageLayout oldImageLayout,
        const VkImageLayout newImageLayout,
        const VkPipelineStageFlags srcStageMask,
        const VkPipelineStageFlags dstStageMask,
        const VkImageSubresourceRange subresourceRange) {
    VkImageMemoryBarrier imageMemoryBarrier = (VkImageMemoryBarrier){
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = srcAccessMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldImageLayout,
            .newLayout = newImageLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = *image,
            .subresourceRange = subresourceRange,
    };

    core_assert(newImageLayout != VK_IMAGE_LAYOUT_UNDEFINED);

    gfx_log_verbose("barrier before: \noldLayout = \t%s \nsrcAccessMask = %s \nsrcStageMask = \t%s\n", VkImageLayout_c_str(oldImageLayout), VkAccessFlagBits_c_str(srcAccessMask) , VkPipelineStageFlags_c_str(srcStageMask));
    gfx_log_verbose("barrier after : \nnewLayout = \t%s \ndstAccessMask = %s \ndstStageMask = \t%s\n\n", VkImageLayout_c_str(newImageLayout), VkAccessFlagBits_c_str(dstAccessMask) , VkPipelineStageFlags_c_str(dstStageMask));
    vkCmdPipelineBarrier(cmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

static void gfx_render_frame(VkCommandBuffer cmdBuffer) {
    constexpr VkPipelineStageFlags PIPELINE_STAGE = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    constexpr uint32_t WAIT_SEMAPHORE_COUNT = 1;
    constexpr uint32_t WAIT_SEMAPHORE_PRESENT_DONE_INDEX = 0;
    VkSemaphoreSubmitInfo waitSemaphores[WAIT_SEMAPHORE_COUNT];
    waitSemaphores[WAIT_SEMAPHORE_PRESENT_DONE_INDEX] = (VkSemaphoreSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = s_gfx.semaphores[gfx_last_swap_chain_index()].presentDone,
        .stageMask = PIPELINE_STAGE,
    };

    constexpr uint32_t SIGNAL_SEMAPHORE_COUNT = 1;
    constexpr uint32_t SIGNAL_SEMAPHORE_RENDER_DONE_INDEX = 0;
    VkSemaphoreSubmitInfo signalSemaphores[SIGNAL_SEMAPHORE_COUNT];
    signalSemaphores[SIGNAL_SEMAPHORE_RENDER_DONE_INDEX] = (VkSemaphoreSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = s_gfx.semaphores[gfx_swap_chain_index()].renderDone,
        .stageMask = PIPELINE_STAGE,
    };

    const VkCommandBufferSubmitInfo cmdBufferInfo = (VkCommandBufferSubmitInfo){
        .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmdBuffer,
    };

    const VkSubmitInfo2 submitInfo = {
        .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount   = WAIT_SEMAPHORE_COUNT,
        .pWaitSemaphoreInfos      = waitSemaphores,
        .commandBufferInfoCount   = 1,
        .pCommandBufferInfos      = &cmdBufferInfo,
        .signalSemaphoreInfoCount = SIGNAL_SEMAPHORE_COUNT,
        .pSignalSemaphoreInfos    = signalSemaphores,
    };

    const VkFence waitFence = s_gfx.graphicsFenceWait[gfx_swap_chain_index()];

    const VkResult submitRes = gfx_queue_submit_2( g_gfx.queue, 1, &submitInfo, waitFence );
    core_assert(submitRes == VK_SUCCESS);
}

static bool gfx_check_window_needs_resize(VkResult result){
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        s_gfx.windowNeedsResize = true;
    } else {
        core_assert_msg(result == VK_SUCCESS, "err: failed to acquire swapchain");
    }
    return s_gfx.windowNeedsResize;
}
//=============================================================================

//===API=======================================================================
void gfx_update(){
    if(s_gfx.windowNeedsResize){
        if(!query_has_valid_extent_size()){
            return;
        }
        gfx_window_resize();
    }

    vkWaitForFences(g_gfx.device, 1, &s_gfx.graphicsFenceWait[gfx_swap_chain_index()], true, UINT64_MAX);
    {
        s_gfx.swapChain.lastImageIndex = gfx_swap_chain_index();
        VkResult swapchainAcquireResult = gfx_acquire_next_swap_chain_image();
        gfx_check_window_needs_resize(swapchainAcquireResult);
    }
    vkResetFences(g_gfx.device, 1, &s_gfx.graphicsFenceWait[gfx_swap_chain_index()]);

    gfx_task_graph_build();

    VkCommandBuffer cmdBuffer = s_gfx.commandBuffers[gfx_buffer_index()];
    vkResetCommandBuffer(cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );

    gfx_begin_command_recording(cmdBuffer);
    {
        gfx_task_graph_run(cmdBuffer);
    }
    gfx_end_command_recording(cmdBuffer);

    gfx_render_frame(cmdBuffer);

    VkResult presentResult = gfx_present();
    gfx_check_window_needs_resize(presentResult);
    s_gfx.currentGfxFrame++;
}


void gfx_vulkan_clear_depth_run(VkCommandBuffer cmdBuffer, GfxRenderContext ctx){
    //===WRITE=================================================================
    core_assert(ctx.writeCount == 2);
    core_assert(ctx.write[0].type == GFX_RESOURCE_IMAGE_COLOR);
    core_assert(ctx.write[0].data.imageColor.binding == 0);
    const GfxImage* writeImage0 = gfx_pool_get_gfx_image(ctx.write[0].data.imageColor.handle);
    //=========================================================================
    const VkViewport viewport = {0, 0, (float)writeImage0->size.x, (float)writeImage0->size.y, 0.0f, 1.0f};
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    const VkRect2D scissor = {{0, 0}, {writeImage0->size.x, writeImage0->size.y}};
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
}

void gfx_vulkan_lit_run(VkCommandBuffer cmdBuffer, GfxRenderContext ctx){
    //===WRITE=================================================================
    core_assert(ctx.writeCount == 2);
    core_assert(ctx.write[0].type == GFX_RESOURCE_IMAGE_COLOR);
    core_assert(ctx.write[0].data.imageColor.binding == 0);
    core_assert(ctx.write[1].type == GFX_RESOURCE_IMAGE_DEPTH_STENCIL);
    core_assert(ctx.write[1].data.imageColor.binding == 1);
    //=========================================================================
    gfx_lit_draw(cmdBuffer);
}

void gfx_vulkan_imgui_run(VkCommandBuffer cmdBuffer, GfxRenderContext ctx){
    //===WRITE=================================================================
    core_assert(ctx.writeCount == 2);
    core_assert(ctx.write[0].type == GFX_RESOURCE_IMAGE_COLOR);
    core_assert(ctx.write[0].data.imageColor.binding == 0);
    //=========================================================================
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
    gfx_imgui_draw(cmdBuffer);
#endif // CHECK_FEATURE(FEATURE_GFX_IMGUI)
}

GfxImage gfx_get_current_swapchain_image_data(){
    const uint32_t currentIndex = gfx_swap_chain_index();
    return (GfxImage){
        .image = s_gfx.swapChain.images[currentIndex],
        .view = s_gfx.swapChain.views[currentIndex],
        .alloc = VK_NULL_HANDLE,
        .size = {.x = s_gfx.swapChain.width, .y = s_gfx.swapChain.height}
    };
}

void gfx_vulkan_blit_image_to_swapchain_run( VkCommandBuffer cmdBuffer, GfxRenderContext ctx ) {
    //===READ==================================================================
    core_assert(ctx.readCount == 1);
    core_assert(ctx.read[0].type == GFX_RESOURCE_IMAGE_COLOR);
    core_assert(ctx.read[0].data.imageColor.binding == 0);
    const GfxImage* readImage0 = gfx_pool_get_gfx_image(ctx.read[0].data.imageColor.handle);
    //=========================================================================
    //===WRITE=================================================================
    core_assert(ctx.writeCount == 1);
    core_assert(ctx.write[0].type == GFX_RESOURCE_IMAGE_EXTERNAL_CB);
    core_assert(ctx.write[0].data.imageColor.binding == 0);
    const GfxImage writeImage0 = ctx.write[0].data.external.imageCB();
    //=========================================================================
    const VkImageBlit blitRegion = {
            .srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1},
            .srcOffsets = {
                    {.x = 0, .y = 0, .z = 0},
                    {.x = (int32_t)(readImage0->size.x), .y = (int32_t)(readImage0->size.y), .z = 1}
            },
            .dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1},
            .dstOffsets = {
                    {.x = 0, .y = 0, .z = 0},
                    {.x = (int32_t)(writeImage0.size.x), .y = (int32_t)( writeImage0.size.y), .z = 1}
            },
    };

    vkCmdBlitImage(
            cmdBuffer,
            readImage0->image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            writeImage0.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blitRegion,
            VK_FILTER_LINEAR
    );
}

void gfx_vulkan_prepare_present_run(VkCommandBuffer /*cmdBuffer*/, GfxRenderContext ctx){
    //===WRITE=================================================================
    core_assert(ctx.readCount == 1);
    core_assert(ctx.read[0].type == GFX_RESOURCE_IMAGE_EXTERNAL_CB);
    core_assert(ctx.read[0].data.imageColor.binding == 0);
    //=========================================================================
}
//===API/GFX_VULKAN_INTERFACE==================================================
vec2u gfx_vulkan_swapchain_size(){
    return (vec2u){{s_gfx.swapChain.width,s_gfx.swapChain.height}};
}

bool gfx_has_drawable_surface(){
    return s_gfx.swapChain.width != 0 || s_gfx.swapChain.height != 0;
};
//=============================================================================

//===API/GFX_VULKAN_INTERFACE/UTILS============================================
VkFormat gfx_vulkan_utils_find_depth_format(const VkImageTiling desiredTilingFormat) {
    constexpr VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    constexpr uint32_t FORMAT_CANDIDATE_COUNT = 5;
    VkFormat candidates[FORMAT_CANDIDATE_COUNT] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM,
    };

    for (uint32_t i = 0; i < FORMAT_CANDIDATE_COUNT; ++i) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(g_gfx.physicalDevice, candidates[i], &props);

        if ((desiredTilingFormat == VK_IMAGE_TILING_LINEAR) && ((props.linearTilingFeatures & features) == features)) {
            return candidates[i];
        }
        if ((desiredTilingFormat == VK_IMAGE_TILING_OPTIMAL) && ((props.optimalTilingFeatures & features) == features)) {
            return candidates[i];
        }
    }
    core_assert_msg(false, "err: could not find supported depth format");
    return VK_FORMAT_UNDEFINED;
}

VkSurfaceFormatKHR gfx_vulkan_utils_select_surface_format() {
    DynamicArray deviceSurfaceFormats = dynamic_array_create(gfx_allocator_arena(), sizeof(vkGetPhysicalDeviceSurfaceFormatsKHR), 0);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_gfx.physicalDevice, s_gfx.swapChain.surface, &deviceSurfaceFormats.current, nullptr);
    dynamic_array_resize(gfx_allocator_arena(), &deviceSurfaceFormats, deviceSurfaceFormats.current);

    VkSurfaceFormatKHR *surfaceFormats = dynamic_array_buffer(&deviceSurfaceFormats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_gfx.physicalDevice, s_gfx.swapChain.surface, &deviceSurfaceFormats.count, surfaceFormats);

    // Try select best surface format
    constexpr VkSurfaceFormatKHR KAH_TARGET_SWAPCHAIN_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    VkSurfaceFormatKHR selectedSurfaceFormat = {};

    if (deviceSurfaceFormats.count == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
        selectedSurfaceFormat = KAH_TARGET_SWAPCHAIN_FORMAT;
    }
    else {
        for (uint32_t i = 0; i < deviceSurfaceFormats.count; ++i) {
            if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                selectedSurfaceFormat = surfaceFormats[i];
                break;
            }
        }
    }
    return selectedSurfaceFormat;
}

VkPhysicalDeviceMemoryProperties gfx_get_device_memory_properties()
{
    core_assert(s_gfx.deviceMemoryProperties.memoryTypeCount != 0 && s_gfx.deviceMemoryProperties.memoryHeapCount != 0);
    return s_gfx.deviceMemoryProperties;
}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_create(void* windowHandle){
#if CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)
    gfx_converter_create(CONVERTER_SRC_ASSRT_DIR , fs_exe_dir());
#endif //CHECK_FEATURE(FEATURE_CONVERT_ON_DEMAND)

    gfx_pool_create();
    gfx_data_structures_create();
    gfx_volk_create();
    gfx_instance_create();
    gfx_debug_callbacks_create();
    gfx_physical_device_create();
    gfx_surface_create(windowHandle, &g_gfx.instance, &s_gfx.swapChain.surface);
    gfx_physical_device_queues_create();
    gfx_vma_create();
    gfx_command_pool_create();
    gfx_semaphores_create();
    gfx_swap_chain_create();
    gfx_command_buffers_create();
    gfx_fences_create();
    gfx_task_graph_create(true);
    gfx_pipeline_cache_create();
    gfx_bindless_create();

#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
    gfx_imgui_create(windowHandle);
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)
    gfx_lit_create();
    gfx_texture_builtin_create();
    gfx_mesh_builtin_create();
}

void gfx_cleanup(){
    gfx_flush();

    gfx_mesh_builtin_cleanup();
    gfx_texture_builtin_cleanup();
    gfx_lit_cleanup();
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
    gfx_imgui_cleanup();
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)

    gfx_bindless_cleanup();
    gfx_pipeline_cache_cleanup();
    gfx_task_graph_cleanup(true);
    gfx_fences_cleanup();
    gfx_command_buffers_cleanup();
    gfx_swap_chain_cleanup();
    gfx_semaphores_cleanup();
    gfx_command_pool_cleanup();
    gfx_vma_cleanup();
    gfx_physical_device_queues_cleanup();
    gfx_surface_cleanup();
    gfx_debug_callbacks_cleanup();
    gfx_instance_cleanup();
    gfx_volk_cleanup();
    gfx_data_structures_cleanup();
    gfx_pool_cleanup();
}
//=============================================================================

