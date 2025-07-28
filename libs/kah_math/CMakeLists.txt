project(kah_math C)

add_library(kah_math
        inc/kah_math/utils.h
        inc/kah_math/vec2.h
        inc/kah_math/vec3.h
        inc/kah_math/vec4.h

        src/utils.c
        src/vec4..c
)

target_include_directories(kah_math
        PUBLIC inc
        PRIVATE inc/kah_math
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_core PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_core PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_core PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_core PUBLIC NDEBUG=1)
endif ()
