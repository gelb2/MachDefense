cmake_minimum_required(VERSION 3.20)
project(MachDefenseWeb LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "Use emcmake with an installed Emscripten SDK")
endif()
option(MD_WEB_CHEAT "Build isolated validation application" OFF)
get_filename_component(MD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
include(${MD_ROOT}/Modules/core-sources.cmake)
add_executable(game ${MACHDEFENSE_CORE_SOURCES} src/Main.cpp src/Renderer.cpp src/Assets.cpp src/Audio.cpp ${MD_ROOT}/Sources/Audio/OpenALEffects.cpp)
target_include_directories(game PRIVATE ${MD_ROOT}/Sources ${MD_ROOT}/Modules/ThirdParty/stb ${MD_ROOT}/Modules/ThirdParty/openal-soft/include src)
target_compile_options(game PRIVATE -Wall -Wextra -ffp-contract=off -fno-fast-math -fexceptions)
target_compile_definitions(game PRIVATE MD_WEB_CHEAT=$<BOOL:${MD_WEB_CHEAT}>)
set_target_properties(game PROPERTIES SUFFIX ".js" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist")
target_link_options(game PRIVATE -fexceptions -sDISABLE_EXCEPTION_CATCHING=0 -sASYNCIFY=1 -sASYNCIFY_STACK_SIZE=131072
 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=134217728 -sSTACK_SIZE=2097152
 -sFORCE_FILESYSTEM=1 -sENVIRONMENT=web -sMODULARIZE=1 -sEXPORT_NAME=createMachDefense -lopenal
 "-sEXPORTED_FUNCTIONS=_main,_md_start,_md_tick,_md_touch,_md_scroll,_md_level10_sweep,_md_content_sweep,_md_resize,_md_active,_md_context,_md_debug,_md_ui,_md_command,_md_audio_probe,_md_save_info,_malloc,_free"
 "-sEXPORTED_RUNTIME_METHODS=ccall,FS,UTF8ToString,HEAPU8,stringToNewUTF8"
 "--preload-file=${MD_ROOT}/Resources/Images@/assets/Images" "--preload-file=${MD_ROOT}/Resources/Generated@/assets/Generated"
 "--preload-file=${MD_ROOT}/Resources/Tables@/assets/Tables" "--preload-file=${MD_ROOT}/Resources/RawData@/assets/RawData"
 "--preload-file=${MD_ROOT}/Resources/Audio@/assets/Audio")
