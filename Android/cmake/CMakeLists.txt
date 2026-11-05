cmake_minimum_required(VERSION 3.20)
project(MachDefenseAndroid LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
get_filename_component(MD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
add_subdirectory("${MD_ROOT}" shared-core)
set(LIBTYPE STATIC CACHE STRING "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_BACKEND_WAVE OFF CACHE BOOL "" FORCE)
set(ALSOFT_BACKEND_OBOE OFF CACHE BOOL "" FORCE)
set(ALSOFT_REQUIRE_OPENSL ON CACHE BOOL "" FORCE)
add_subdirectory("${MD_ROOT}/Modules/ThirdParty/openal-soft" openal)
include(opensl-resume.cmake)
add_library(machdefense SHARED
    ../app/src/main/cpp/Bridge.cpp
    ../app/src/main/cpp/Runtime.cpp
    ../app/src/main/cpp/Renderer.cpp
    ../app/src/main/cpp/Assets.cpp)
target_include_directories(machdefense PRIVATE "${MD_ROOT}/Modules/ThirdParty/stb")
target_compile_definitions(machdefense PRIVATE MD_VALIDATION=$<BOOL:${MD_VALIDATION}>)
target_compile_options(machdefense PRIVATE -Wall -Wextra -ffp-contract=off -fno-fast-math)
target_link_libraries(machdefense PRIVATE MachDefenseCore MachDefenseAudio OpenAL EGL GLESv1_CM android log)
target_link_options(machdefense PRIVATE "-Wl,--no-undefined" "-Wl,-z,max-page-size=16384")

# Device validation uses the same optimized game loops as the release app.
foreach(target MachDefenseCore MachDefenseAudio machdefense)
    target_compile_options(${target} PRIVATE -O2)
endforeach()
