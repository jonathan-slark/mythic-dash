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

typedef struct ScreenState {
  int width;
  int height;
  int refreshRate;
  int scale;
} ScreenState;

// --- Constants ---

/// The origin of the maze in the screen.
extern const Vector2 MAZE_ORIGIN;

// --- Global state ---

/// Shared screen state across the game.
extern ScreenState gScreenState;
