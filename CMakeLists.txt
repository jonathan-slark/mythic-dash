cmake_minimum_required(VERSION 3.21)
project(maze_muncher C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(raylib REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")

add_library(log STATIC src/log/log.c)
target_include_directories(log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
)
target_link_libraries(${PROJECT_NAME} raylib
    winmm
    gdi32
    opengl32
    log
)

add_executable(test_log src/test/log.c)
target_include_directories(test_log PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(test_log PRIVATE log)
