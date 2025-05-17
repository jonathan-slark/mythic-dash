cmake_minimum_required(VERSION 3.21)
project(maze_muncher C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define common paths
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(EXTERN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern)

# Common include directories
set(COMMON_INCLUDES
    ${SRC_DIR}
    ${EXTERN_DIR}
)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
find_package(raylib REQUIRED)
file(GLOB SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.c")

# Build log library
add_library(log STATIC ${SRC_DIR}/log/log.c)
target_include_directories(log PUBLIC ${SRC_DIR})

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_INCLUDES})
target_link_libraries(${PROJECT_NAME} 
    raylib
    winmm
    gdi32
    opengl32
    log
)

# Test executable
add_executable(test_log ${SRC_DIR}/test/log.c)
target_include_directories(test_log PRIVATE ${COMMON_INCLUDES})
target_link_libraries(test_log PRIVATE log)
