/// Shared types, constants, and global state across the game.

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
