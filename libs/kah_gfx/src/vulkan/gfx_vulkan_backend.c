//===INCLUDES==================================================================
#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/gfx_logging.h>
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_surface.h>

#include <kah_core/assert.h>
#include <kah_core/dynamic_array.h>
#include <kah_core/c_string.h>
#include <kah_core/utils.h>
#include <kah_core/bit_array.h>

#include <kah_math/utils.h>
#include <kah_math/vec2.h>

#include <stdio.h>
//=============================================================================

//===INTERNAL_CONSTANTS/DEFINES================================================
constexpr uint32_t KAH_SWAP_CHAIN_IMAGE_COUNT = 3;
constexpr uint32_t KAH_BUFFER_COUNT = 3;

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

//HACK: 1.4 SDK is broken for me & fails on debug callbacks.
#define FEATURE_GFX_VK_1_4 FEATURE_OFF
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
struct GfxSemaphores {
    VkSemaphore presentDone;
    VkSemaphore renderDone;
}typedef GfxSemaphores;

struct GfxSwapChainBuffers {
    VkImage image;
    VkImageView view;
} typedef GfxSwapChainBuffers;

struct GfxImage {
    VkImage image;
    VkImageView view;
    VmaAllocation alloc;
} typedef GfxImage;

struct GfxBuffer {
    VkBuffer buffer;
    VkBufferView view;
    VmaAllocation alloc;
} typedef GfxBuffer;

struct GfxSwapChain {
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;

    uint32_t imageCount;
    uint32_t currentImageIndex;
    uint32_t lastImageIndex;
    VkImage images[KAH_SWAP_CHAIN_IMAGE_COUNT];
    GfxSwapChainBuffers buffers[KAH_SWAP_CHAIN_IMAGE_COUNT];

    uint32_t width;
    uint32_t height;
}typedef GfxSwapChain;

static struct GfxBackend{
    uint32_t instanceVersion;

    VmaAllocator allocator;

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    VkInstance instance;

    GfxSwapChain swapChain;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[KAH_SWAP_CHAIN_IMAGE_COUNT];
    VkFence graphicsFenceWait[KAH_SWAP_CHAIN_IMAGE_COUNT];
    GfxSemaphores semaphores[KAH_SWAP_CHAIN_IMAGE_COUNT];

    struct{
        GfxImage color;
        GfxImage depthStencil;
    } backBuffer;

    VkAllocationCallbacks* allocCallback;

    VkSampleCountFlagBits sampleCount;

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
} s_gfx = {};

static struct GfxDebug {
    VkDebugUtilsMessengerEXT debugUtilsMessenger;
} s_gfxDebug = {};

static struct GfxUserArguments {
    uint32_t selectedPhysicalDeviceIndex;
    bool vsync;
    VkSampleCountFlagBits msaa;
} s_userArguments = {};

static struct GfxFeatures{
    VkPhysicalDeviceFeatures2 deviceFeatures;
    VkPhysicalDeviceVulkan11Features features11;
    VkPhysicalDeviceVulkan12Features features12;
    VkPhysicalDeviceVulkan13Features features13;
    // VkPhysicalDeviceVulkan14Features features14;

    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchainFeatures;
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicState1Features;
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT dynamicState2Features;
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features;
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures;
} s_gfxFeatures = {};

static struct TargetVulkanFormats {
    VkSurfaceFormatKHR surfaceFormat;
    VkFormat depthFormat;
    VkFormat colorFormat;
} s_targetVulkanFormats = {};
//=============================================================================

//===INTERNAL_FUNCTIONS========================================================
Allocator gfx_allocator(){ return allocators()->cstd;}          // TODO: replace with gfx lifetime arena allocator
Allocator gfx_allocator_arena(){ return allocators()->arena;}   // TODO: replace with gfx frame arena

static void gfx_create_data_structures(){
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
#if CHECK_FEATURE(FEATURE_GFX_VK_1_4)
        .features14 =               (VkPhysicalDeviceVulkan14Features){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES } ,
#endif // #if CHECK_FEATURE(FEATURE_GFX_VK_1_4)
        .swapchainFeatures =        (VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT } ,
        .dynamicState1Features =    (VkPhysicalDeviceExtendedDynamicStateFeaturesEXT){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT } ,
        .dynamicState2Features =    (VkPhysicalDeviceExtendedDynamicState2FeaturesEXT){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT } ,
        .dynamicState3Features =    (VkPhysicalDeviceExtendedDynamicState3FeaturesEXT){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT } ,
        .dynamicRenderingFeatures = (VkPhysicalDeviceDynamicRenderingFeaturesKHR){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR }
    };

    s_gfx.supportedInstanceExtensions = dynamic_array_create(gfx_allocator(), sizeof(VkExtensionProperties),0);
    s_gfx.supportedValidationLayers = dynamic_array_create(gfx_allocator(), sizeof(VkLayerProperties), 0);
    s_gfx.supportedphysicalDevices = dynamic_array_create(gfx_allocator(), sizeof(VkPhysicalDevice), 0);
    s_gfx.supportedDeviceExtensions = dynamic_array_create(gfx_allocator(), sizeof(VkExtensionProperties), 0);
}

static void gfx_cleanup_data_structures(){
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
        .physicalDevice = s_gfx.physicalDevice,
        .device = s_gfx.device ,
        .pAllocationCallbacks = s_gfx.allocCallback,
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
            .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements
        },
        .instance = s_gfx.instance,
        // .vulkanApiVersion = ,
        // .preferredLargeHeapBlockSize = ,
        // .pDeviceMemoryCallbacks = ,
        // .pHeapSizeLimit = ,
        // .pTypeExternalMemoryHandleTypes =
    };

    const VkResult vmaResult = vmaCreateAllocator( &allocatorInfo, &s_gfx.allocator );
    core_assert_msg(vmaResult == VK_SUCCESS, "err: failed to init VMA");
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
    return UINT32_MAX;
}


static uint32_t gfx_find_supported_device_extension_index(const char* extensionName){
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

    const VkResult result = vkCreateInstance(&instanceInfo, s_gfx.allocCallback, &s_gfx.instance);
    core_assert_msg(result == VK_SUCCESS, "err: failed to create vulkan instance");

    volkLoadInstance(s_gfx.instance); // load instnace function pointers
    core_assert(vkDestroyInstance != VK_NULL_HANDLE); // validate callbacks are setup
    //=========================================================================
}

static void gfx_instance_cleanup(){
    core_assert_msg(s_gfx.instance != VK_NULL_HANDLE, "err: VkInstance has already been destroyed");
    vkDestroyInstance(s_gfx.instance, s_gfx.allocCallback);
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
    vkCreateDebugUtilsMessengerEXT(s_gfx.instance, &messengerCreateInfo, s_gfx.allocCallback, &s_gfxDebug.debugUtilsMessenger);
}

static void gfx_debug_callbacks_cleanup() {
    core_assert_msg(s_gfxDebug.debugUtilsMessenger != VK_NULL_HANDLE, "err: debug utils messenger has already been destroyed");
    vkDestroyDebugUtilsMessengerEXT(s_gfx.instance, s_gfxDebug.debugUtilsMessenger, s_gfx.allocCallback);
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
             totalDeviceLocalMem / (float)KAH_MiB,
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
    VkSampleCountFlags supportedSampleCount = min_i32(deviceProperties.limits.framebufferColorSampleCounts, deviceProperties.limits.framebufferDepthSampleCounts);
    return (supportedSampleCount & target) ? target : VK_SAMPLE_COUNT_1_BIT;
}

static void gfx_physical_device_create(){
    core_assert(s_gfx.supportedphysicalDevices.current == 0);
    vkEnumeratePhysicalDevices(s_gfx.instance, &s_gfx.supportedphysicalDevices.current, nullptr);
    core_assert_msg(s_gfx.supportedphysicalDevices.current != 0, "err: did not find any vulkan compatible physical devices");
    dynamic_array_resize(gfx_allocator(),&s_gfx.supportedphysicalDevices, s_gfx.supportedphysicalDevices.current);

    VkPhysicalDevice* physicalDevices = dynamic_array_buffer(&s_gfx.supportedphysicalDevices);
    vkEnumeratePhysicalDevices(s_gfx.instance, &s_gfx.supportedphysicalDevices.count, physicalDevices);

    debug_print_supported_physical_devices_info();

    // TODO:GFX: Add fallback support for `best` GPU based on intended workload, if no argument is provided we fallback to device [0]
    uint32_t selectedDevice = s_userArguments.selectedPhysicalDeviceIndex;
    core_assert_msg(selectedDevice < s_gfx.supportedphysicalDevices.count, "err: selecting physical vulkan device that is out of range");
    s_gfx.physicalDevice = physicalDevices[selectedDevice];

    vkGetPhysicalDeviceProperties(s_gfx.physicalDevice, &s_gfx.deviceProperties);
    s_gfx.sampleCount = set_target_sample_count(s_gfx.deviceProperties, s_userArguments.msaa);

    vkGetPhysicalDeviceFeatures(s_gfx.physicalDevice, &s_gfx.deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(s_gfx.physicalDevice, &s_gfx.deviceMemoryProperties);

    debug_print_selected_physical_device_info();
}

static void gfx_surface_cleanup() {
    // gfx_create_surface(...) exists in gfx_vulkan_surface.h / gfx_vulkan_surface_windows.cpp
    core_assert_msg(s_gfx.swapChain.surface != VK_NULL_HANDLE, "err: VkSurface has already been destroyed");
    vkDestroySurfaceKHR(s_gfx.instance, s_gfx.swapChain.surface, s_gfx.allocCallback);
    s_gfx.swapChain.surface = VK_NULL_HANDLE;
}

static void gfx_physical_device_queues_create(){
    vkEnumerateDeviceExtensionProperties(s_gfx.physicalDevice, nullptr, &s_gfx.supportedDeviceExtensions.current, nullptr);
    dynamic_array_resize(gfx_allocator(), &s_gfx.supportedDeviceExtensions, s_gfx.supportedDeviceExtensions.current);
    VkExtensionProperties* supportedDeviceExtensions = dynamic_array_buffer(&s_gfx.supportedDeviceExtensions);

    if (s_gfx.supportedDeviceExtensions.count > 0) {
        vkEnumerateDeviceExtensionProperties(s_gfx.physicalDevice, nullptr, &s_gfx.supportedDeviceExtensions.count, supportedDeviceExtensions);
    }

    for (uint32_t i = 0; i < s_gfx.supportedDeviceExtensions.count; ++i) {
        gfx_log_verbose("Supported device extensions: %s \n", supportedDeviceExtensions[i].extensionName);
    }

    DynamicArray selectedQueueFamilies = dynamic_array_create(gfx_allocator_arena(), sizeof(VkQueueFamilyProperties),0);
    vkGetPhysicalDeviceQueueFamilyProperties(s_gfx.physicalDevice, &selectedQueueFamilies.current, nullptr);
    dynamic_array_resize(gfx_allocator_arena(), &selectedQueueFamilies,selectedQueueFamilies.current);

    VkQueueFamilyProperties* queueFamilies = dynamic_array_buffer(&selectedQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(s_gfx.physicalDevice, &selectedQueueFamilies.count, queueFamilies);

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
        VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(s_gfx.physicalDevice,queueFamilyIndex,s_gfx.swapChain.surface,&supportsPresent);

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

    const float graphicsQueuePriority = 1.0f;
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = graphicsQueueInfo.queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &graphicsQueuePriority
    };

    DynamicArray usedDeviceExtensions = dynamic_array_create(gfx_allocator_arena(), sizeof(const char*), 7);

    const char* swapChainName        = gfx_find_supported_device_extension_name( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    const char* dynamicRenderingName = gfx_find_supported_device_extension_name( VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME );
    const char* pushDescriptor       = gfx_find_supported_device_extension_name( VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME );
    const char* extDynamicState1     = gfx_find_supported_device_extension_name( VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME );
    const char* extDynamicState2     = gfx_find_supported_device_extension_name( VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME );
    const char* extDynamicState3     = gfx_find_supported_device_extension_name( VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME );
    const char* swapchainMaintence1  = gfx_find_supported_device_extension_name( VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME );

    s_gfxFeatures.deviceFeatures.pNext = &s_gfxFeatures.features11;
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,2,0)){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.features12);
    }
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,3,0)){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.features13);
    }
#if CHECK_FEATURE(FEATURE_GFX_VK_1_4)
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,4,0)){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.features14);
    }
#endif // #if CHECK_FEATURE(FEATURE_GFX_VK_1_4)

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
    if( dynamicRenderingName && s_gfx.deviceProperties.apiVersion < VK_MAKE_VERSION(1,3,0) ){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicRenderingFeatures);
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &dynamicRenderingName);
    }

    vkGetPhysicalDeviceFeatures2(s_gfx.physicalDevice, &s_gfxFeatures.deviceFeatures);
    core_assert_msg(s_gfxFeatures.features12.descriptorIndexing, "err: Descriptor indexing is required");
    core_assert_msg(s_gfxFeatures.features12.bufferDeviceAddress, "err: Buffer device address is required");
    core_assert_msg(s_gfxFeatures.features13.dynamicRendering || s_gfxFeatures.dynamicRenderingFeatures.dynamicRendering, "err: Dynamic rendering is required");

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

    vkCreateDevice(s_gfx.physicalDevice, &deviceCreateInfo, s_gfx.allocCallback, &s_gfx.device);
    core_assert_msg(s_gfx.device, "err: Failed to create vkDevice");

    vkGetDeviceQueue(s_gfx.device, graphicsQueueInfo.queueIndex, 0, &s_gfx.queue);
    core_assert_msg(s_gfx.queue, "err: Failed to create graphics queue");

    s_gfx.queueFamilyIndex =  graphicsQueueInfo.queueIndex;
}

static void gfx_physical_device_queues_cleanup(){
    vkDestroyDevice(s_gfx.device, s_gfx.allocCallback);
    s_gfx.device = VK_NULL_HANDLE;
    s_gfx.queue = VK_NULL_HANDLE;
}

static void gfx_command_pool_create(){
    VkCommandPoolCreateInfo commandPoolInfo = (VkCommandPoolCreateInfo){
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex =  s_gfx.queueFamilyIndex,
    };
    const VkResult result = vkCreateCommandPool(s_gfx.device, &commandPoolInfo, s_gfx.allocCallback, &s_gfx.commandPool);
    core_assert_msg(result == VK_SUCCESS, "err: failed to create graphics command pool");
}

static void gfx_command_pool_cleanup(){
    vkDestroyCommandPool(s_gfx.device, s_gfx.commandPool, s_gfx.allocCallback);
    s_gfx.commandPool = VK_NULL_HANDLE;
}

static void gfx_semaphores_create(){
    for (uint32_t i = 0; i < KAH_SWAP_CHAIN_IMAGE_COUNT; ++i) {
        VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        const VkResult presentRes = vkCreateSemaphore(s_gfx.device, &semaphoreInfo, s_gfx.allocCallback, &s_gfx.semaphores[i].presentDone);
        const VkResult renderRes = vkCreateSemaphore(s_gfx.device, &semaphoreInfo, s_gfx.allocCallback, &s_gfx.semaphores[i].renderDone);
        core_assert_msg(presentRes == VK_SUCCESS, "err: failed to create present semaphore");
        core_assert_msg(renderRes  == VK_SUCCESS, "err: failed to create render semaphore");
    }
}


static void gfx_semaphores_cleanup(){
    for (uint32_t i = 0; i < KAH_SWAP_CHAIN_IMAGE_COUNT; ++i) {
        vkDestroySemaphore(s_gfx.device, s_gfx.semaphores[i].presentDone, s_gfx.allocCallback);
        vkDestroySemaphore(s_gfx.device, s_gfx.semaphores[i].renderDone, s_gfx.allocCallback);
    }
}


static VkPresentModeKHR select_present_mode() {
    DynamicArray devicePresentModes = dynamic_array_create(gfx_allocator_arena(), sizeof(VkPresentModeKHR), 0);
    const VkResult presentRes = vkGetPhysicalDeviceSurfacePresentModesKHR(s_gfx.physicalDevice, s_gfx.swapChain.surface, &devicePresentModes.current, nullptr);
    core_assert_msg(presentRes == VK_SUCCESS, "err: failed to get physical device surface present modes");
    core_assert(devicePresentModes.current > 0);
    dynamic_array_resize(gfx_allocator_arena(), &devicePresentModes, devicePresentModes.current);

    VkPresentModeKHR* presentModes = dynamic_array_buffer(&devicePresentModes);
    const VkResult populateRes = vkGetPhysicalDeviceSurfacePresentModesKHR(s_gfx.physicalDevice, s_gfx.swapChain.surface, &devicePresentModes.count, presentModes);
    core_assert_msg(populateRes == VK_SUCCESS, "err: failed to populate present modes array");

    VkPresentModeKHR selectedPresentMode = {VK_PRESENT_MODE_FIFO_KHR};
    VkPresentModeKHR preferredMode = s_userArguments.vsync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < devicePresentModes.count; ++i) {
        if (presentModes[i] == preferredMode) {
            selectedPresentMode = presentModes[i];
            break;
        }
    }
    return selectedPresentMode;
}


VkSurfaceFormatKHR gfx_utils_select_surface_format() {
    DynamicArray deviceSurfaceFormats = dynamic_array_create(gfx_allocator_arena(), sizeof(vkGetPhysicalDeviceSurfaceFormatsKHR), 0);
    vkGetPhysicalDeviceSurfaceFormatsKHR(s_gfx.physicalDevice, s_gfx.swapChain.surface, &deviceSurfaceFormats.current, nullptr);
    dynamic_array_resize(gfx_allocator_arena(), &deviceSurfaceFormats, deviceSurfaceFormats.current);

    VkSurfaceFormatKHR *surfaceFormats = dynamic_array_buffer(&deviceSurfaceFormats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(s_gfx.physicalDevice, s_gfx.swapChain.surface, &deviceSurfaceFormats.count, surfaceFormats);

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

VkFormat gfx_utils_find_depth_format(const VkImageTiling desiredTilingFormat) {
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
        vkGetPhysicalDeviceFormatProperties(s_gfx.physicalDevice, candidates[i], &props);

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

static void gfx_swap_chain_create(){
    const VkSwapchainKHR oldSwapChain = s_gfx.swapChain.swapChain;

    VkSurfaceCapabilitiesKHR surfaceCapabilities = {0};
    const VkResult surfRes = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_gfx.physicalDevice, s_gfx.swapChain.surface, &surfaceCapabilities);
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
    s_targetVulkanFormats.surfaceFormat = gfx_utils_select_surface_format();
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
        .imageFormat = s_targetVulkanFormats.surfaceFormat.format,
        .imageColorSpace = s_targetVulkanFormats.surfaceFormat.colorSpace,
        .imageExtent = swapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = swapchainImageUsage,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = 0,
        .preTransform = surfaceTransform,
        .compositeAlpha = compositeAlphaFormat,
        .presentMode = swapChainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapChain
    };

    const VkResult swapChainRes = vkCreateSwapchainKHR(s_gfx.device, &swapChainInfo, s_gfx.allocCallback, &s_gfx.swapChain.swapChain);
    core_assert_msg(swapChainRes == VK_SUCCESS, "err: failed to create swap chain");

    //cleanup vulkan resources
    if (oldSwapChain != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; i++) {
            vkDestroyImageView(s_gfx.device, s_gfx.swapChain.buffers[i].view, s_gfx.allocCallback);
        }
        vkDestroySwapchainKHR(s_gfx.device, oldSwapChain, s_gfx.allocCallback);
    }

    vkGetSwapchainImagesKHR(s_gfx.device, s_gfx.swapChain.swapChain, &s_gfx.swapChain.imageCount, nullptr);
    VkResult swapChainImageResult = vkGetSwapchainImagesKHR(s_gfx.device,  s_gfx.swapChain.swapChain, &s_gfx.swapChain.imageCount, &s_gfx.swapChain.images[0] );
    core_assert_msg(swapChainImageResult == VK_SUCCESS, "err: failed to re-create swap chain images");

    VkComponentMapping         components;
    VkImageSubresourceRange    subresourceRange;

    for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; i++) {
        VkImageViewCreateInfo swapChainImageViewInfo = (VkImageViewCreateInfo){
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr ,
            .flags = 0,
            .image = nullptr,
            .viewType = VK_IMAGE_VIEW_TYPE_2D ,
            .format = s_targetVulkanFormats.surfaceFormat.format,
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

        s_gfx.swapChain.buffers[i].image = s_gfx.swapChain.images[i];
        swapChainImageViewInfo.image = s_gfx.swapChain.buffers[i].image;

        const VkResult imageViewResult = vkCreateImageView(s_gfx.device, &swapChainImageViewInfo, s_gfx.allocCallback, &s_gfx.swapChain.buffers[i].view);
        core_assert_msg(imageViewResult == VK_SUCCESS, "err: Failed to create image view %u", i);
    }
}

static void gfx_swap_chain_cleanup(){
    for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; i++) {
        vkDestroyImageView(s_gfx.device, s_gfx.swapChain.buffers[i].view, s_gfx.allocCallback);
    }

    core_assert_msg(s_gfx.swapChain.surface != VK_NULL_HANDLE,"err: swapchain surface has already been destroyed");
    vkDestroySwapchainKHR(s_gfx.device, s_gfx.swapChain.swapChain, s_gfx.allocCallback);
    s_gfx.swapChain.swapChain = VK_NULL_HANDLE;
}

static void gfx_command_buffers_create(){
    const VkCommandBufferAllocateInfo gfxCommandBufferInfo = (VkCommandBufferAllocateInfo){
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = s_gfx.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = KAH_SWAP_CHAIN_IMAGE_COUNT,
    };

    VkResult cmdBuffResult = vkAllocateCommandBuffers(s_gfx.device, &gfxCommandBufferInfo, s_gfx.commandBuffers);
    core_assert_msg(cmdBuffResult == VK_SUCCESS, "err: Failed to create graphics command buffer");

    //Consider adding upload cmd buffer.
}

static void gfx_command_buffers_cleanup(){
    vkFreeCommandBuffers(s_gfx.device, s_gfx.commandPool, KAH_SWAP_CHAIN_IMAGE_COUNT, s_gfx.commandBuffers);
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
        const VkResult fenceRes = vkCreateFence(s_gfx.device, &fenceCreateInfo, s_gfx.allocCallback, &s_gfx.graphicsFenceWait[i]);
        core_assert_msg(fenceRes == VK_SUCCESS, "err: Failed to create graphics fence [%u]", i);
    }
}

static void gfx_fences_cleanup() {
    for (uint32_t i = 0; i < s_gfx.swapChain.imageCount; ++i) {
        vkDestroyFence(s_gfx.device, s_gfx.graphicsFenceWait[i], s_gfx.allocCallback);
        s_gfx.graphicsFenceWait[i] = VK_NULL_HANDLE;
    }
}

static void gfx_color_buffer_create(){
    s_targetVulkanFormats.colorFormat = s_targetVulkanFormats.surfaceFormat.format; // Consider adding a new find best format function
    VkImageCreateInfo colourImageInfo = (VkImageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = s_targetVulkanFormats.colorFormat,
        .extent = (VkExtent3D){
            .width = s_gfx.swapChain.width,
            .height = s_gfx.swapChain.height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = s_gfx.sampleCount,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocInfo = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT
    };

    const VkResult createImgRes = vmaCreateImage(s_gfx.allocator, &colourImageInfo, &allocInfo, &s_gfx.backBuffer.color.image, &s_gfx.backBuffer.color.alloc, nullptr);
    core_assert(createImgRes == VK_SUCCESS);

    VkResult bindRes = vmaBindImageMemory( s_gfx.allocator, s_gfx.backBuffer.color.alloc, s_gfx.backBuffer.color.image );
    core_assert_msg(bindRes == VK_SUCCESS, "err: Failed to bind color buffer");

    VkImageViewCreateInfo colorImageViewInfo = (VkImageViewCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = s_gfx.backBuffer.color.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = s_targetVulkanFormats.colorFormat,
        .subresourceRange = (VkImageSubresourceRange){
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    const VkResult createRes = vkCreateImageView(s_gfx.device, &colorImageViewInfo, s_gfx.allocCallback, &s_gfx.backBuffer.color.view);
    core_assert_msg(createRes == VK_SUCCESS, "err: Failed to create color image view");
}

void gfx_image_free(GfxImage* image){
    vmaDestroyImage(s_gfx.allocator, image->image, image->alloc);
    vkDestroyImageView(s_gfx.device, image->view, s_gfx.allocCallback);
}

static void gfx_color_buffer_cleanup(){
    gfx_image_free(&s_gfx.backBuffer.color);
}

static void gfx_depth_stencil_buffer_create() {
    s_targetVulkanFormats.depthFormat = gfx_utils_find_depth_format(VK_IMAGE_TILING_OPTIMAL);
    const VkImageCreateInfo depthImageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = s_targetVulkanFormats.depthFormat,
            .extent = {
                    .width = s_gfx.swapChain.width,
                    .height = s_gfx.swapChain.height,
                    .depth =  1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = s_gfx.sampleCount,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    };

    VmaAllocationCreateInfo allocInfo = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .flags = VMA_ALLOCATION_CREATE_DONT_BIND_BIT,
    };

    const VkResult createImgRes = vmaCreateImage(s_gfx.allocator, &depthImageInfo, &allocInfo, &s_gfx.backBuffer.depthStencil.image, &s_gfx.backBuffer.depthStencil.alloc, nullptr);
    core_assert(createImgRes == VK_SUCCESS);

    VkResult bindRes = vmaBindImageMemory( s_gfx.allocator, s_gfx.backBuffer.depthStencil.alloc, s_gfx.backBuffer.depthStencil.image );
    core_assert_msg(bindRes == VK_SUCCESS, "err: Failed to bind depth stencil buffer");

    VkImageViewCreateInfo depthImageViewInfo = (VkImageViewCreateInfo){
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = s_gfx.backBuffer.depthStencil.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = s_targetVulkanFormats.depthFormat,
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            }
    };

    if (s_targetVulkanFormats.depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        depthImageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    const VkResult createRes = vkCreateImageView(s_gfx.device, &depthImageViewInfo, nullptr, &s_gfx.backBuffer.depthStencil.view);
    core_assert_msg(createRes == VK_SUCCESS, "err: Failed to create depth image view");
}

static void gfx_depth_stencil_buffer_cleanup(){
    gfx_image_free(&s_gfx.backBuffer.depthStencil);
}

static void gfx_pipeline_cache_create(){
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    const VkResult cacheRes = vkCreatePipelineCache(s_gfx.device, &pipelineCacheCreateInfo, s_gfx.allocCallback, &s_gfx.pipelineCache);
    core_assert_msg(cacheRes == VK_SUCCESS, "err: Failed to create pipeline cache");
}

static void gfx_pipeline_cache_cleanup(){
    vkDestroyPipelineCache(s_gfx.device, s_gfx.pipelineCache, s_gfx.allocCallback);
}

static uint32_t gfx_swap_chain_index() {
    return (s_gfx.swapChain.currentImageIndex);
}

static uint32_t gfx_last_swap_chain_index() {
    return (s_gfx.swapChain.lastImageIndex);
}

uint32_t gfx_buffer_index() {
    return (s_gfx.currentGfxFrame % KAH_BUFFER_COUNT);
}

static vec2i gfx_screen_size() {
    return (vec2i){ .x = s_gfx.swapChain.width, .y = s_gfx.swapChain.height };
}

static VkResult gfx_acquire_next_swap_chain_image() {
    return vkAcquireNextImageKHR(
            s_gfx.device,
            s_gfx.swapChain.swapChain,
            UINT64_MAX,
            s_gfx.semaphores[gfx_last_swap_chain_index()].presentDone,
            VK_NULL_HANDLE,
            &s_gfx.swapChain.currentImageIndex
    );
}


static bool query_has_valid_extent_size() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_gfx.physicalDevice, s_gfx.swapChain.surface, &surfaceCapabilities);
    return surfaceCapabilities.currentExtent.width != 0 || surfaceCapabilities.currentExtent.height != 0;
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

    return vkQueuePresentKHR(s_gfx.queue, &presentInfo);
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

static void gfx_flush() {
    if (s_gfx.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(s_gfx.device);
    }
}

static void gfx_window_resize() {
    if (!query_has_valid_extent_size()) {
        return;
    }
    gfx_flush();

    gfx_color_buffer_cleanup();
    gfx_depth_stencil_buffer_cleanup();
    gfx_swap_chain_cleanup();

    gfx_swap_chain_create();
    gfx_color_buffer_create();
    gfx_depth_stencil_buffer_create();
}

void gfx_command_begin_immediate_recording() {
    // add immediate cmd buf instead of reusing the current queue
    // add validation that cmd buf isn't already in use
    VkCommandBufferBeginInfo cmdBufBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(s_gfx.commandBuffers[gfx_buffer_index()], &cmdBufBeginInfo);
}

void gfx_command_end_immediate_recording() {
    // add immediate cmd buf
    // add validation that cmd buf isn't already in use
    vkEndCommandBuffer(s_gfx.commandBuffers[gfx_buffer_index()]);

    VkSubmitInfo submitInfo = (VkSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &s_gfx.commandBuffers[gfx_buffer_index()],
    };
    //TODO:GFX replace immediate submit with transfer immediate submit.
    vkQueueSubmit(s_gfx.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(s_gfx.queue);
}

void gfx_command_begin_rendering(VkCommandBuffer cmdBuffer, const VkRenderingInfoKHR *renderingInfo) {
    //TODO: we should resolve this to KAH engine specific FP
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,3,0)){
        vkCmdBeginRendering(cmdBuffer, renderingInfo);
    }
    else{
        vkCmdBeginRenderingKHR(cmdBuffer, renderingInfo);
    }
}

void gfx_command_end_rendering(VkCommandBuffer cmdBuffer) {
    //TODO: we should resolve this to KAH engine specific FP
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,3,0)){
        vkCmdEndRendering(cmdBuffer);
    }
    else{
        vkCmdEndRenderingKHR(cmdBuffer);
    }
}

void gfx_command_insert_memory_barrier(
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
    vkCmdPipelineBarrier(cmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void gfx_dynamic_render(VkCommandBuffer cmdBuffer){
    gfx_command_insert_memory_barrier(
            cmdBuffer,
            &s_gfx.swapChain.buffers[gfx_swap_chain_index()].image,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            (VkImageSubresourceRange){VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1,}
    );
    {
        const VkRenderingAttachmentInfoKHR colorAttachment = (VkRenderingAttachmentInfoKHR){
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = s_gfx.swapChain.buffers[gfx_swap_chain_index()].view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {.color = {{0.5f, 0.092f, 0.167f, 1.0f}},},
        };

        const VkRenderingAttachmentInfoKHR depthStencilAttachment = (VkRenderingAttachmentInfoKHR){
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = s_gfx.backBuffer.depthStencil.view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {.depthStencil = {.depth = 1.0f, .stencil = 0}},
        };

        const VkRenderingInfoKHR renderingInfo = (VkRenderingInfoKHR){
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {.offset = {0, 0}, .extent = {s_gfx.swapChain.width, s_gfx.swapChain.height}},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment,
            .pDepthAttachment = &depthStencilAttachment,
            .pStencilAttachment = &depthStencilAttachment,
        };

        gfx_command_begin_rendering(cmdBuffer, &renderingInfo);
        {
            const VkViewport viewport = {0, 0, (float)s_gfx.swapChain.width, (float)s_gfx.swapChain.height, 0.0f, 1.0f};
            vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

            const VkRect2D scissor = {0, 0, s_gfx.swapChain.width, s_gfx.swapChain.height};
            vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
        }
        gfx_command_end_rendering(cmdBuffer);
    }
    gfx_command_insert_memory_barrier(
            cmdBuffer,
            &s_gfx.swapChain.buffers[gfx_swap_chain_index()].image,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            (VkImageSubresourceRange){VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    );
}

static void gfx_render_frame(VkCommandBuffer *cmdBuffer) {
    constexpr VkPipelineStageFlags submitPipelineStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &s_gfx.semaphores[gfx_last_swap_chain_index()].presentDone,
        .pWaitDstStageMask = &submitPipelineStages,
        .commandBufferCount = 1,
        .pCommandBuffers = cmdBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &s_gfx.semaphores[gfx_swap_chain_index()].renderDone,
    };

    const VkResult submitRes = vkQueueSubmit(s_gfx.queue, 1, &submitInfo, s_gfx.graphicsFenceWait[gfx_swap_chain_index()]);
    core_assert(submitRes == VK_SUCCESS);
}
//=============================================================================

//===API=======================================================================
void gfx_update(){
    s_gfx.swapChain.lastImageIndex = gfx_swap_chain_index();
    const VkResult nextRes = gfx_acquire_next_swap_chain_image();
    if (nextRes == VK_ERROR_OUT_OF_DATE_KHR) {
        gfx_window_resize();
        return;
    } else if (nextRes < 0) {
        core_assert(nextRes == VK_SUCCESS);
    }
    vkWaitForFences(s_gfx.device, 1, &s_gfx.graphicsFenceWait[gfx_swap_chain_index()], true, UINT64_MAX);
    vkResetFences(s_gfx.device, 1, &s_gfx.graphicsFenceWait[gfx_swap_chain_index()]);
    VkCommandBuffer cmdBuffer = s_gfx.commandBuffers[gfx_buffer_index()];
    vkResetCommandBuffer(cmdBuffer, 0);

    gfx_begin_command_recording(cmdBuffer);
    {
        gfx_dynamic_render(cmdBuffer); //Replace with render graph execute here.
    }
    gfx_end_command_recording(cmdBuffer);

    gfx_render_frame(&cmdBuffer);
    gfx_present();
    gfx_flush();

    s_gfx.currentGfxFrame++;
}

//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_create(void* windowHandle){
    gfx_create_data_structures();
    gfx_volk_create();
    gfx_instance_create();
    gfx_debug_callbacks_create();
    gfx_physical_device_create();
    gfx_surface_create(windowHandle, &s_gfx.instance, &s_gfx.swapChain.surface);
    gfx_physical_device_queues_create();
    gfx_vma_create();
    gfx_command_pool_create();
    gfx_semaphores_create();
    gfx_swap_chain_create();
    gfx_command_buffers_create();
    gfx_fences_create();
    gfx_color_buffer_create();
    gfx_depth_stencil_buffer_create();
    gfx_pipeline_cache_create();
}

void gfx_cleanup(){
    gfx_pipeline_cache_cleanup();
    gfx_depth_stencil_buffer_cleanup();
    gfx_color_buffer_cleanup();
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
    gfx_cleanup_data_structures();
}
//=============================================================================

