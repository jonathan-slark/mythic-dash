// clang-format Language: C
#pragma once

#include <raylib.h>  // Vector2

#define GAME_TRY(func) \
  if (!(func)) {       \
    return false;      \
  }

// --- Constants ---

extern const Vector2 MAZE_ORIGIN;

// --- Player functions ---

void game__playerInit(void);
void game__playerUpdate(void);
Vector2 game__playerGetPos(void);
