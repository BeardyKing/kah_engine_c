project(kah_core C)

add_library(kah_core
        inc/kah_core/memory.h
        src/memory.c
        inc/kah_core/allocators.h
        inc/kah_core/assert.h
        src/allocators.c
)

target_include_directories(kah_core
        PUBLIC inc
        PRIVATE inc/kah_core
)

#target_link_libraries(kah_core
#        kah_math
#)

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