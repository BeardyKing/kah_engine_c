project(kah_game C)

add_executable(kah_game
        main.c
)

set_target_properties(kah_game PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/runtime")

target_link_libraries(kah_game
        kah_math
        kah_core
        kah_gfx
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_game PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_game PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_game PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_game PUBLIC NDEBUG=1)
endif ()


target_compile_options(kah_converter PRIVATE
#===SUPPORTED TARGETS==========================================================
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<PLATFORM_ID:MinGW>>:
#==============================================================================
#===DISABLE====================================================================
    -Wno-old-style-declaration
#==============================================================================
#===ENABLE=====================================================================
    -Werror
    -Wall
    -Wextra
    -Wunused-variable
    -Wreturn-type
    -Wconversion
    -Wsign-conversion
    -Wfloat-conversion
    -Wnarrowing
    -Wreturn-local-addr
    -Wuninitialized
    -Wshadow
    -Wpointer-arith
    -Wint-to-pointer-cast
    -Wbad-function-cast
#==============================================================================
    >
)