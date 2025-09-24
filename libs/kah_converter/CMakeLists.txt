project(kah_converter C)

add_library(kah_converter
        inc/kah_converter/converter_interface.h
        src/converter_backend.c
        src/converter_shader.c
        src/converter_cache.c
        src/converter_texture.c
)

target_include_directories(kah_converter
        PUBLIC inc
        PRIVATE inc/kah_converter
)

target_link_libraries(kah_converter
        kah_core
)

set_target_properties(kah_converter PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_converter PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_converter PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_converter PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_converter PUBLIC NDEBUG=1)
endif ()
