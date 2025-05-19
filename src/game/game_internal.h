// clang-format Language: C
#pragma once

#include <raylib.h>  // Vector2

// --- Macros ---

#define GAME_TRY(func) \
  if (!(func)) {       \
    return false;      \
  }

// --- Player functions ---

void game__playerInit(void);
void game__playerUpdate(void);
Vector2 game__playerGetPos(void);
