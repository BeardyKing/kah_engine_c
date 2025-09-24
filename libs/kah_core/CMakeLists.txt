project(kah_core C)

add_library(kah_core
        inc/kah_core/memory.h
        src/memory.c
        inc/kah_core/allocators.h
        inc/kah_core/assert.h
        src/allocators.c
        inc/kah_core/defines.h
        inc/kah_core/fixed_array.h
        src/fixed_array.c
        inc/kah_core/window.h
        src/window_win32.c
        inc/kah_core/input_types.h
        inc/kah_core/input.h
        src/input.c
        inc/kah_core/time.h
        src/time_win32.c
        src/time_linux.c
        inc/kah_core/bit_array.h
        inc/kah_core/utils.h
        src/bit_array.c
        src/assert.c
        inc/kah_core/tests/test_bits.h
        src/tests/test_bits.c
        src/tests/test_data_structures.c
        inc/kah_core/tests/test_data_structures.h
        inc/kah_core/dynamic_array.h
        src/dynamic_array.c
        inc/kah_core/c_string.h
        src/c_string.c
        inc/kah_core/logging.h
        inc/kah_core/filesystem.h
        src/filesystem_win32.c
        inc/kah_core/dds_loader.h
        src/dds_loader.c
        inc/kah_core/texture_formats.h
)

target_include_directories(kah_core
        PUBLIC inc
        PRIVATE inc/kah_core
)

target_link_libraries(kah_core
        kah_math
)

set_target_properties(kah_core PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)


if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_core PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_core PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_core PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_core PUBLIC NDEBUG=1)
endif ()

# Conditional inclusion of X11 and XFixes for Linux builds
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED x11)
    pkg_check_modules(XFIXES REQUIRED xfixes)

    include_directories(
            ${X11_INCLUDE_DIRS}
            ${XFIXES_INCLUDE_DIRS}
    )

    target_link_libraries(kah_core
            ${X11_LIBRARIES}
            ${XFIXES_LIBRARIES}
    )
endif ()