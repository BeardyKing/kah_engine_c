project(kah_gfx C)
add_library(kah_gfx
        inc/kah_gfx/gfx_interface.h
        inc/kah_gfx/vulkan/gfx_vulkan_surface.h

        src/vulkan/gfx_vulkan_backend.c
        src/vulkan/gfx_vulkan_surface_win.c
        inc/kah_gfx/vulkan/gfx_vulkan.h
        src/vulkan/gfx_vulkan.c
        inc/kah_gfx/gfx_logging.h
        src/gfx_logging.c
        inc/kah_gfx/vulkan/gfx_vulkan_imgui.h
        src/vulkan/gfx_vulkan_imgui.c
        inc/kah_gfx/vulkan/gfx_vulkan_interface.h
        inc/kah_gfx/vulkan/gfx_vulkan_types.h
        inc/kah_gfx/gfx_task_graph.h
        src/gfx_task_graph.c
        inc/kah_gfx/gfx_pool.h
        src/gfx_pool.c
        inc/kah_gfx/vulkan/gfx_vulkan_resource.h
        src/vulkan/gfx_vulkan_resource.c
        inc/kah_gfx/vulkan/gfx_vulkan_utils.h
        src/vulkan/gfx_vulkan_utils.c
        inc/kah_gfx/vulkan/gfx_vulkan_lit.h
        src/vulkan/gfx_vulkan_lit.c
        inc/kah_gfx/vulkan/gfx_vulkan_shader.h
        src/vulkan/gfx_vulkan_shader.c
        inc/kah_gfx/gfx_converter.h
        src/gfx_converter.c
        inc/kah_gfx/vulkan/gfx_vulkan_bindless.h
        src/vulkan/gfx_vulkan_bindless.c
        inc/kah_gfx/vulkan/gfx_vulkan_texture.h
        src/vulkan/gfx_vulkan_texture.c
        inc/kah_gfx/vulkan/gfx_vulkan_mesh.h
        src/vulkan/gfx_vulkan_mesh.c
        inc/kah_gfx/vulkan/gfx_vulkan_buffer.h
        src/vulkan/gfx_vulkan_buffer.c
        inc/kah_gfx/vulkan/gfx_vulkan_pipeline_builder.h
        src/vulkan/gfx_vulkan_pipeline_builder.c
        inc/kah_gfx/gfx_types.h
        inc/kah_gfx/vulkan/gfx_vulkan_uniform_buffers.h
        src/gfx_vulkan_uniform_buffers.c
)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    target_compile_definitions(kah_gfx PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
    target_compile_definitions(kah_gfx PRIVATE VK_USE_PLATFORM_XLIB_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    target_compile_definitions(kah_gfx PRIVATE VK_USE_PLATFORM_MACOS_MVK)
endif()

#if(NOT WIN32)
#    target_link_libraries(kah_gfx PRIVATE dl)
#endif()

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
)

find_package(Vulkan REQUIRED)
target_link_libraries(kah_gfx
        kah_core
        kah_math
        kah_converter

        volk
        volk_headers
        vma_static
        dcimgui
)

set_target_properties(kah_gfx PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

target_compile_definitions(kah_gfx PRIVATE VK_NO_PROTOTYPES)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_gfx PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_gfx PUBLIC KAH_RELEASE=0)
    target_compile_definitions(kah_gfx PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_gfx PUBLIC KAH_DEBUG=0)
    target_compile_definitions(kah_gfx PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_gfx PUBLIC NDEBUG=1)
endif ()

#target_compile_options(kah_gfx PRIVATE
##===SUPPORTED TARGETS==========================================================
#    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<PLATFORM_ID:MinGW>>:
##==============================================================================
##===DISABLE====================================================================
#     #-Wno-old-style-declaration
#    -Wno-declaration-after-statement
#    -Wno-pre-c23-compat
#    -Wno-unsafe-buffer-usage
#    -Wno-error=float-equal
##==============================================================================
##===ENABLE=====================================================================
#    -Werror
#    -Wall
#    -Wextra
#    -Wunused-variable
#    -Wreturn-type
#    -Wconversion
#    -Wsign-conversion
#    -Wfloat-conversion
#    -Wnarrowing
#    -Wreturn-local-addr
#    -Wuninitialized
#    -Wshadow
#    -Wpointer-arith
#    -Wint-to-pointer-cast
#    -Wbad-function-cast
##==============================================================================
#    >
#)
