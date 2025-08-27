cmake_minimum_required(VERSION 3.11)
include(FetchContent)

#===SLANG======================================================================
find_package(Vulkan REQUIRED)
if(NOT Vulkan_SLANGC_EXECUTABLE)
    get_filename_component(_Vulkan_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
    find_program(Vulkan_SLANGC_EXECUTABLE
            NAMES slangc
            HINTS ${_Vulkan_LIB_DIR}/../Bin
    )
endif()
#==============================================================================

#===VOLK=======================================================================
FetchContent_Declare(
        volk
        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG        1.3.295
)
FetchContent_MakeAvailable(volk)
#==============================================================================

#===VMA========================================================================
#NOTE: VulkanMemoryAllocator uses C++ 14 implementation W/ C API
FetchContent_Declare(
        VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG        v3.1.0
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

add_library(vma_static STATIC vma_static/vma_static.cpp)
target_include_directories(vma_static PUBLIC ${vulkanmemoryallocator_SOURCE_DIR}/include)

find_package(Vulkan REQUIRED)
target_link_libraries(vma_static PUBLIC Vulkan::Vulkan)
#==============================================================================

#===DEAR_IMGUI=================================================================
#NOTE: dear imgui uses C++ API & implementation.
#      Checked in dear imgui version grabbed from release src "v1.92.2b-docking"
#      https://github.com/ocornut/imgui/releases/tag/v1.92.2b-docking
#
#      Bindings built locally from release src "DearBindings_v0.16_ImGui_v1.92.2b"
#      https://github.com/dearimgui/dear_bindings/releases/tag/DearBindings_v0.16_ImGui_v1.92.2b
project(dcimgui)
add_library(dcimgui
        dear_c_imgui/imgui.h
        dear_c_imgui/imgui.cpp
        dear_c_imgui/imgui_demo.cpp
        dear_c_imgui/imgui_draw.cpp
        dear_c_imgui/imgui_widgets.cpp
        dear_c_imgui/imgui_tables.cpp
        dear_c_imgui/backends/imgui_impl_vulkan.cpp
        dear_c_imgui/backends/imgui_impl_vulkan.h

        dear_c_imgui/dcimgui.cpp
        dear_c_imgui/dcimgui.h
        dear_c_imgui/dcimgui_internal.cpp
        dear_c_imgui/dcimgui_internal.h
        dear_c_imgui/backends/dcimgui_impl_vulkan.cpp
        dear_c_imgui/backends/dcimgui_impl_vulkan.h
)

if (WIN32)
    list(APPEND IMGUI_BACKENDS
            dear_c_imgui/backends/imgui_impl_win32.cpp
            dear_c_imgui/backends/imgui_impl_win32.h

            dear_c_imgui/backends/dcimgui_impl_win32.cpp
            dear_c_imgui/backends/dcimgui_impl_win32.h
    )
endif ()

find_package(Vulkan REQUIRED)
target_link_libraries(dcimgui
        Vulkan::Vulkan
        volk
)

if (WIN32)
target_link_libraries( dcimgui
        dwmapi
)
endif()

target_include_directories(dcimgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dear_c_imgui)
target_include_directories(dcimgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dear_c_imgui/backends)
target_compile_definitions(dcimgui PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)
target_sources(dcimgui PRIVATE ${IMGUI_BACKENDS})
