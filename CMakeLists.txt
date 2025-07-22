cmake_minimum_required(VERSION 3.11)
project(kah_engine_c)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(KAH_CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#====THIRD_PARTY_LIBS=====
#add_subdirectory(libs/third_party)

#====LIBS=================
add_subdirectory(libs/kah_core)
#add_subdirectory(libs/kah_math)
#add_subdirectory(libs/kah_gfx)
#add_subdirectory(libs/kah_converter)

#====PROJECTS=============
add_subdirectory(projects/kah_game)

set(CMAKE_C_STANDARD 23)

