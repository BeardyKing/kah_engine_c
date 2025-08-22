//===INCLUDES==================================================================
#include <kah_gfx/gfx_interface.h>
#include <kah_gfx/vulkan/gfx_vulkan.h>
#include <kah_gfx/vulkan/gfx_vulkan_surface.h>

#include <kah_core/assert.h>
#include <kah_core/dynamic_array.h>
#include <kah_core/c_string.h>
#include <kah_core/utils.h>
#include <kah_core/bit_array.h>

#include <kah_math/utils.h>

#include <stdio.h>
//=============================================================================

//===INTERNAL_STRUCTS==========================================================
static struct GfxBackend{
    uint32_t instanceVersion;

    VmaAllocator allocator;

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkAllocationCallbacks* allocCallback;

    VkSampleCountFlagBits sampleCount;

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

    DynamicArray supportedInstanceExtensions;
    DynamicArray supportedValidationLayers;
    DynamicArray supportedphysicalDevices;
    DynamicArray supportedDeviceExtensions;
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
        // .features14 =               (VkPhysicalDeviceVulkan14Features){.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES } ,
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
        printf("Supported instance extension: %s \n", supportedExtension[i].extensionName);
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
        printf("Used instance extension: %s\n", extName);
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
        printf("Layer: %s - Desc: %s\n", supportedValidationLayers[i].layerName, supportedValidationLayers->description);
    }

    char* validationLayerName = ((VkLayerProperties*)dynamic_array_get(&s_gfx.supportedValidationLayers,gfx_find_supported_validation_layer(KAH_VK_VALIDATION_LAYER_NAME)))->layerName;

    DynamicArray usedValidationLayers = dynamic_array_create(gfx_allocator_arena(), sizeof(char*), 1);
    dynamic_array_push(gfx_allocator_arena(), &usedValidationLayers, &validationLayerName);

    for (uint32_t i = 0; i < usedValidationLayers.current; ++i) {
        char* str = *(char**)dynamic_array_get(&usedValidationLayers, i);
        printf("Used validation layers: %s \n", str);
    }
    //=========================================================================

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "KAH_ENGINE",
        .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
        .pEngineName = "KAH_ENGINE",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_MAKE_VERSION(1,4,0), //vulkan 1.2 support required
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
            printf("[ERROR]: \ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            printf("[WARNING]:\ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            printf("[INFO]: \ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
            break;
        }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            printf("[VERBOSE]: \ncode: \t\t%s \nmessage: \t%s\n", callbackData->pMessageIdName, callbackData->pMessage);
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

    printf("=== PHYSICAL DEVICE INFO ======\n\n");
    printf("Found %u Vulkan device(s):\n", s_gfx.supportedphysicalDevices.count);
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

        printf("Device %u: %s\n", i, props.deviceName);
        printf("\tType   : %s\n", device_type_to_string(props.deviceType));
        printf("\tMemory : %.2f MiB of device-local memory\n", (float)totalDeviceLocalMem / (float)KAH_MiB);
        printf("\n");
    }
    printf("===============================\n");
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

    printf("Selected physical device\n");
    printf("Name:\t\t%s \nType:\t\t%s \nMemory:\t\t%.2f MiB \nHeap Count:\t%u \nVersion:\t%u.%u.%u \nDriver: \t%u.%u.%u\n",
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
    core_assert_msg(s_gfx.surface != VK_NULL_HANDLE, "err: VkSurface has already been destroyed");
    vkDestroySurfaceKHR(s_gfx.instance, s_gfx.surface, s_gfx.allocCallback);
    s_gfx.surface = VK_NULL_HANDLE;
}

static void gfx_queues_create(){
    vkEnumerateDeviceExtensionProperties(s_gfx.physicalDevice, nullptr, &s_gfx.supportedDeviceExtensions.current, nullptr);
    dynamic_array_resize(gfx_allocator(), &s_gfx.supportedDeviceExtensions, s_gfx.supportedDeviceExtensions.current);
    VkExtensionProperties* supportedDeviceExtensions = dynamic_array_buffer(&s_gfx.supportedDeviceExtensions);

    if (s_gfx.supportedDeviceExtensions.count > 0) {
        vkEnumerateDeviceExtensionProperties(s_gfx.physicalDevice, nullptr, &s_gfx.supportedDeviceExtensions.count, supportedDeviceExtensions);
    }

    for (uint32_t i = 0; i < s_gfx.supportedDeviceExtensions.count; ++i) {
        printf("Supported device extensions: %s \n", supportedDeviceExtensions[i].extensionName);
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

    QueueInfo graphicsQueueInfo = (QueueInfo){
        .targetFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
        .queueIndex = UINT32_MAX,
        .currentFlags = UINT32_MAX,
        .supportsPresent = false
    };

    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < selectedQueueFamilies.count; ++queueFamilyIndex){
        VkBool32 supportsPresent = VK_FALSE;
        VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(s_gfx.physicalDevice,queueFamilyIndex,s_gfx.surface,&supportsPresent);

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
    printf("Graphics queue index: %u \n", graphicsQueueInfo.queueIndex);
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

    DynamicArray usedDeviceExtensions = dynamic_array_create(gfx_allocator_arena(), sizeof(const char*), 10);
    const char* swapChainName = gfx_find_supported_device_extension_name(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &swapChainName);

    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,2,0)){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.features12);
    }
    else {
        // TODO: VK1.1 fallback support for descriptorIndexing & bufferDeviceAddress
    }
    if(s_gfx.deviceProperties.apiVersion >= VK_MAKE_VERSION(1,3,0)){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.features13);
    }
    else {
        const char* dynamicRenderingName = gfx_find_supported_device_extension_name(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        if( dynamicRenderingName ){
            pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicRenderingFeatures);
            dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &dynamicRenderingName);
        }
    }
    const char* pushDescriptor = gfx_find_supported_device_extension_name(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    if(pushDescriptor){
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &pushDescriptor);
    }
    const char* extDynamicState1 = gfx_find_supported_device_extension_name(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    if(extDynamicState1){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicState1Features);
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &extDynamicState1);
    }
    const char* extDynamicState2 = gfx_find_supported_device_extension_name(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
    if(extDynamicState2){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicState2Features);
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &extDynamicState2);
    }
    const char* extDynamicState3 = gfx_find_supported_device_extension_name(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
    if(extDynamicState3){
        pnext_chain_push_front(&s_gfxFeatures.features11, &s_gfxFeatures.dynamicState3Features);
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &extDynamicState3);
    }
    const char* swapchainMaintence1 = gfx_find_supported_device_extension_name(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
    if(swapchainMaintence1){
        dynamic_array_push(gfx_allocator_arena(), &usedDeviceExtensions, &swapchainMaintence1);
    }

    s_gfxFeatures.deviceFeatures.pNext = &s_gfxFeatures.features11;
    vkGetPhysicalDeviceFeatures2(s_gfx.physicalDevice, &s_gfxFeatures.deviceFeatures);
    core_assert_msg(s_gfxFeatures.features12.descriptorIndexing, "err: Descriptor indexing is required");
    core_assert_msg(s_gfxFeatures.features12.bufferDeviceAddress, "err: Buffer device address is required");
    core_assert_msg(s_gfxFeatures.features13.dynamicRendering || s_gfxFeatures.dynamicRenderingFeatures.dynamicRendering, "err: Dynamic rendering is required");

    const char** extName = dynamic_array_buffer(&usedDeviceExtensions);
    for (uint32_t i = 0; i < usedDeviceExtensions.current; ++i){
        printf("Selected device extensions: %s \n", extName[i]);
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
}

static void gfx_queues_cleanup(){
    vkDestroyDevice(s_gfx.device, s_gfx.allocCallback);
    s_gfx.device = VK_NULL_HANDLE;
    s_gfx.queue = VK_NULL_HANDLE;
}
//=============================================================================

//===API=======================================================================
void gfx_update(){}
//=============================================================================

//===INIT/SHUTDOWN=============================================================
void gfx_create(void* windowHandle){
    gfx_create_data_structures();
    gfx_volk_create();
    gfx_instance_create();
    gfx_debug_callbacks_create();
    gfx_physical_device_create();
    gfx_surface_create(windowHandle, &s_gfx.instance, &s_gfx.surface);
    gfx_queues_create();

    VmaAllocatorCreateInfo info = {};
    // vma_create(info);
    printf("Vulkan(volk) version %d.%d.%d initialized.\n",VK_VERSION_MAJOR(s_gfx.instanceVersion),VK_VERSION_MINOR(s_gfx.instanceVersion),VK_VERSION_PATCH(s_gfx.instanceVersion));
}

void gfx_cleanup(){
    gfx_queues_cleanup();
    gfx_surface_cleanup();
    gfx_debug_callbacks_cleanup();
    gfx_instance_cleanup();
    // vma_cleanup();
    gfx_volk_cleanup();
    gfx_cleanup_data_structures();
}
//=============================================================================

