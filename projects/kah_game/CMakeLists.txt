project(kah_game C)

add_executable(kah_game
        main.c
)

target_link_libraries(kah_game
        kah_math
        kah_core
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(kah_game PUBLIC KAH_DEBUG=1)
    target_compile_definitions(kah_game PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(kah_game PUBLIC KAH_RELEASE=1)
    target_compile_definitions(kah_game PUBLIC NDEBUG=1)
endif ()