project(kah_math C)

add_library(kah_math
        inc/kah_math/utils.h
        inc/kah_math/vec2.h
        inc/kah_math/vec3.h
        inc/kah_math/vec4.h

        src/utils.c
        src/vec4..c
        inc/kah_math/defines.h
)

target_include_directories(kah_math
        PUBLIC inc
        PRIVATE inc/kah_math
)

set_target_properties(kah_math PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_math PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_math PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_math PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_math PUBLIC NDEBUG=1)
endif ()


#target_compile_options(kah_math PRIVATE
##===SUPPORTED TARGETS==========================================================
#    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<PLATFORM_ID:MinGW>>:
##==============================================================================
##===DISABLE====================================================================
#    #-Wno-old-style-declaration
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