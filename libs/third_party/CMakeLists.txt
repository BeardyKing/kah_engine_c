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

#===IMGUI======================================================================
#TODO: use cimgui bindings
#FetchContent_Declare(
#        imgui
#        GIT_REPOSITORY https://github.com/ocornut/imgui.git
#        GIT_TAG docking
#        GIT_SHALLOW TRUE
#)
#FetchContent_MakeAvailable(imgui)
#target_compile_definitions(kah_gfx PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)
#==============================================================================


