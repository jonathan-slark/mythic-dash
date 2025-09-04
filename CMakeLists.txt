cmake_minimum_required(VERSION 3.21)
project(mythic-dash LANGUAGES C)

add_subdirectory(extern/engine)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PLATFORM Web CACHE STRING "Platform for raylib")
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

set(CUTE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/cute_headers/include)
set(RAYLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/raylib)
set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/engine/include)
set(MINUNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/minunit/include)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# --- Main executable ---

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${SRC_DIR}/*.c)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${ENGINE_DIR} ${RAYLIB_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${CUTE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC winmm gdi32 opengl32)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib engine log)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:Debug>:ASSET_DIR="../../asset/">
  $<$<CONFIG:Release>:ASSET_DIR="./asset/">
)

# Disable console window
target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-mwindows>)

# For cute_tiled.h
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-Wno-alloc-size-larger-than>)

# --- Test game executable ---

set(TEST_GAME test_game)
add_executable(${TEST_GAME} ${TEST_DIR}/game.c)
target_include_directories(${TEST_GAME} PUBLIC ${MINUNIT_DIR})
target_include_directories(${TEST_GAME} PRIVATE ${TEST_DIR})
target_link_libraries(${TEST_GAME} PRIVATE engine log)

# --- Test level scaling ---

set(TEST_SCALE test_scale)
add_executable(${TEST_SCALE} ${TEST_DIR}/scale.c)

# --- Package a release ---

add_custom_target(package_release ALL COMMENT "Packaging game for distribution")
add_custom_command(TARGET package_release POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/package
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/mythic-dash.exe ${CMAKE_BINARY_DIR}/package/mythic-dash.exe
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/asset ${CMAKE_BINARY_DIR}/package/asset
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/package ${CMAKE_COMMAND} -E tar cfv ../mythic-dash.zip --format=zip -- .
  COMMENT "Created mythic-dash.zip in ${CMAKE_BINARY_DIR}"
)

# --- Web (HTML5) build ---

set(WEB_TARGET ${PROJECT_NAME}_web)

add_executable(${WEB_TARGET} ${SOURCES})
target_include_directories(${WEB_TARGET} PUBLIC ${ENGINE_DIR} ${RAYLIB_DIR})
target_include_directories(${WEB_TARGET} PRIVATE ${INCLUDE_DIR} ${CUTE_DIR})
target_link_libraries(${WEB_TARGET} PRIVATE raylib engine log)
target_compile_definitions(${WEB_TARGET} PRIVATE ASSET_DIR="./asset/")

# Emscripten link flags
set_target_properties(${WEB_TARGET} PROPERTIES
  LINK_FLAGS "-s USE_GLFW=3 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 --preload-file ${CMAKE_SOURCE_DIR}/asset@/asset"
  SUFFIX ".html"
)
