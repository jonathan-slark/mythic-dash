cmake_minimum_required(VERSION 3.21)
project(maze_muncher LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(raylib REQUIRED)
add_subdirectory(extern/engine)

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/engine/include)
set(MINUNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/minunit/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Main executable
file(GLOB SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${ENGINE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
  raylib
  winmm
  gdi32
  opengl32
  engine
  log
)

# Test game executable
set(TEST_GAME test_game)
add_executable(${TEST_GAME} ${TEST_DIR}/test_game.c)
target_include_directories(${TEST_GAME} PUBLIC ${MINUNIT_DIR})
target_include_directories(${TEST_GAME} PRIVATE ${TEST_DIR})
target_link_libraries(${TEST_GAME} PRIVATE engine log)
