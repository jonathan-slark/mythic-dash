cmake_minimum_required(VERSION 3.21)
project(maze_muncher C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
file(GLOB SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.c" "${SRC_DIR}/engine/*.c" "${SRC_DIR}/game/*.c")

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

# Test game executable
file(GLOB SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/game/*.c")
add_executable(test_game ${SRC_DIR}/test/game.c)
target_include_directories(test_game PRIVATE ${COMMON_INCLUDES})
add_subdirectory(extern/engine)
add_subdirectory(extern/log)
target_link_libraries(test_game PRIVATE engine log raylib)

# Doxygen documentation setup
find_package(Doxygen)
if(DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doc)

  add_custom_target(doc ALL
    COMMAND ${CMAKE_COMMAND} -E echo "[Doxygen] Command: ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}"
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )

  if(WIN32)
    add_custom_command(TARGET doc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "[Doxygen] Opening Doxygen HTML docs"
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN} &&
      start "" "${DOXYGEN_OUTPUT_DIR}/html/index.html"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
  elseif(APPLE)
    add_custom_command(TARGET doc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "[Doxygen] Opening Doxygen HTML docs"
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN} &&
      open "${DOXYGEN_OUTPUT_DIR}/html/index.html"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
  elseif(UNIX)
    add_custom_command(TARGET doc POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "[Doxygen] Opening Doxygen HTML docs"
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN} &&
      xdg-open "${DOXYGEN_OUTPUT_DIR}/html/index.html"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
  endif()
else()
  message(STATUS "Doxygen not found. Documentation target will be unavailable.")
endif()

# Custom target to zip the src directory
add_custom_target(zip_source
  COMMAND ${CMAKE_COMMAND} -E echo "[Zip] Creating zip archive of src/"
  COMMAND zip -r ${CMAKE_CURRENT_SOURCE_DIR}/src.zip src
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Zipping the src directory"
  VERBATIM
)
