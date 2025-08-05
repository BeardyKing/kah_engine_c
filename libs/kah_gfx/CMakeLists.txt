project(kah_gfx C)
add_library(kah_gfx
        inc/kah_gfx/gfx_interface.h
        inc/kah_gfx/vulkan/gfx_vulkan_surface.h

        src/vulkan/gfx_vulkan_backend.c
        src/vulkan/gfx_vulkan_surface_win.c
        inc/kah_gfx/vulkan/gfx_vulkan.h
        src/vulkan/gfx_vulkan.c
)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    target_compile_definitions(kah_gfx PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    target_compile_definitions(kah_gfx PRIVATE VK_USE_PLATFORM_XLIB_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_compile_definitions(kah_gfx PRIVATE VK_USE_PLATFORM_MACOS_MVK)
endif()

if(NOT WIN32)
    target_link_libraries(kah_gfx PRIVATE dl)
endif()

find_package(Vulkan QUIET)
if(TARGET Vulkan::Vulkan)
    # Note: We don't use target_link_libraries for Vulkan::Vulkan to avoid a static dependency on libvulkan1
    target_include_directories(kah_gfx PRIVATE ${Vulkan_INCLUDE_DIRS})
elseif(DEFINED ENV{VULKAN_SDK})
    target_include_directories(kah_gfx PRIVATE "$ENV{VULKAN_SDK}/include")
endif()

target_include_directories(kah_gfx
        PUBLIC inc
        PRIVATE src

#        PRIVATE ${VulkanMemoryAllocator_SOURCE_DIR}/include
)


find_package(Vulkan REQUIRED)
target_link_libraries(kah_gfx
        kah_core
        kah_math

        volk
        volk_headers
        vma_static
)

set_target_properties(kah_gfx PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

target_compile_definitions(kah_gfx PRIVATE VK_NO_PROTOTYPES)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_gfx PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_gfx PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_gfx PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_gfx PUBLIC NDEBUG=1)
endif ()
