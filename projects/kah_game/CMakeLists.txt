project(kah_game C)

add_executable(kah_game
        main.c

        inc/client/widgets/widget_manager.h
        src/widgets/widget_manager.c
        inc/client/entity_builder.h
        src/entity_builder.c
)


target_include_directories(kah_core
        PUBLIC inc
        PRIVATE inc/client
)

if(DEFINED EXECUTABLE_SUFFIX)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}_${EXECUTABLE_SUFFIX}"
    )
endif()

target_link_libraries(kah_game
        kah_math
        kah_core
        kah_gfx
)

set_target_properties(kah_game PROPERTIES C_STANDARD 23 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_game PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_game PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_game PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_game PUBLIC NDEBUG=1)
endif ()


#target_compile_options(kah_converter PRIVATE
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