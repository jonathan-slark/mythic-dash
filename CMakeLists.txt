cmake_minimum_required(VERSION 3.21)
project(mythic-dash LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(raylib REQUIRED)
add_subdirectory(extern/engine)

set(CUTE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/cute_headers/include)
set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/engine/include)
set(MINUNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/minunit/include)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Main executable
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${SRC_DIR}/*.c)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${ENGINE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${CUTE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC raylib winmm gdi32 opengl32)
target_link_libraries(${PROJECT_NAME} PRIVATE engine log)

# Test game executable
set(TEST_GAME test_game)
add_executable(${TEST_GAME} ${TEST_DIR}/game.c)
target_include_directories(${TEST_GAME} PUBLIC ${MINUNIT_DIR})
target_include_directories(${TEST_GAME} PRIVATE ${TEST_DIR})
target_link_libraries(${TEST_GAME} PRIVATE engine log)

# Test level scaling
set(TEST_SCALE test_scale)
add_executable(${TEST_SCALE} ${TEST_DIR}/scale.c)

# Package a build
add_custom_target(package ALL
  COMMENT "Packaging game for distribution"
)
add_custom_command(TARGET package POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/package
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/mythic-dash.exe ${CMAKE_BINARY_DIR}/package/mythic-dash.exe
  COMMAND ${CMAKE_COMMAND} -E copy_if_different C:/bin/msys64/ucrt64/bin/libraylib.dll C:/bin/msys64/ucrt64/bin/glfw3.dll ${CMAKE_BINARY_DIR}/package/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/asset ${CMAKE_BINARY_DIR}/package/asset
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/package ${CMAKE_COMMAND} -E tar cfv ../mythic-dash.zip --format=zip -- .
  COMMENT "Created mythic-dash.zip in ${CMAKE_BINARY_DIR}"
)
