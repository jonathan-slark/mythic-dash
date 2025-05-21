// clang-format Language: C
#pragma once

#include <raylib.h>  // Vector2
#include "../log/log.h"

// --- Helper macros ---

#define GAME_TRY(func) \
  if (!(func)) {       \
    return false;      \
  }

// --- Constants ---

extern const Vector2 MAZE_ORIGIN;

// --- Global state ---

extern log_Log* game__log;

// --- Player functions ---

void game__playerInit(void);
void game__playerUpdate(void);
Vector2 game__playerGetPos(void);
