cmake_minimum_required(VERSION 3.21)
project(mythic-dash LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(extern/engine)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PLATFORM Web CACHE STRING "Platform for raylib")
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

if(EMSCRIPTEN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}
  -s USE_GLFW=3 
  -s WASM=1 
  -s ASYNCIFY 
  -s GL_ENABLE_GET_PROC_ADDRESS=1 
  -s EXPORTED_RUNTIME_METHODS=HEAPF32 
  --preload-file ../asset/")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

set(CUTE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/cute_headers/include)
set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/engine/include)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MINUNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/minunit/include)
set(RAYLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/engine/extern/raylib/src)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# --- Main executable ---

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${SRC_DIR}/*.c)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${ENGINE_DIR} ${RAYLIB_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${CUTE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:ASSET_DIR="../../asset/">
  $<$<CONFIG:Release>:ASSET_DIR="./asset/">
)
target_link_libraries(${PROJECT_NAME} PRIVATE engine log)

# Disable console window
target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-mwindows>)

if(!EMSCRIPTEN)
  # For cute_tiled.h
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-Wno-alloc-size-larger-than>)
endif()

# --- Test game executable ---

set(TEST_GAME test_game)
add_executable(${TEST_GAME} EXCLUDE_FROM_ALL ${TEST_DIR}/game.c)
target_include_directories(${TEST_GAME} PUBLIC ${MINUNIT_DIR})
target_include_directories(${TEST_GAME} PRIVATE ${TEST_DIR})
target_link_libraries(${TEST_GAME} PRIVATE engine log)

# --- Test level scaling ---

set(TEST_SCALE test_scale)
add_executable(${TEST_SCALE} EXCLUDE_FROM_ALL ${TEST_DIR}/scale.c)

# --- Package a release ---

add_custom_target(package_release EXCLUDE_FROM_ALL COMMENT "Packaging game for distribution")
add_custom_command(TARGET package_release POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/package
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/mythic-dash.exe ${CMAKE_BINARY_DIR}/package/mythic-dash.exe
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/asset ${CMAKE_BINARY_DIR}/package/asset
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/package ${CMAKE_COMMAND} -E tar cfv ../mythic-dash.zip --format=zip -- .
  COMMENT "Created mythic-dash.zip in ${CMAKE_BINARY_DIR}"
)
