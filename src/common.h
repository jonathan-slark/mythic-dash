/**
 * @file common.h
 * @brief Common types, constants, and global state shared across the game.
 *
 * Defines screen state, global constants, and declarations needed
 * throughout the game source. Intended to be included in most translation units.
 */

// clang-format Language: C
#pragma once

#include <raylib.h>

// --- Types ---

/**
 * @struct ScreenState
 * @brief Stores the current display configuration
 */
typedef struct ScreenState {
  int width;       /**< Width of the screen in pixels */
  int height;      /**< Height of the screen in pixels */
  int refreshRate; /**< Display refresh rate in Hz */
  int scale;       /**< Rendering scale factor */
} ScreenState;

// --- Constants ---

/// @brief The origin of the maze in the screen.
extern const Vector2 MAZE_ORIGIN;

// --- Global state ---

/// @brief Shared screen state across the game.
extern ScreenState gScreenState;
