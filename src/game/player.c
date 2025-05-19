#include "game_internal.h"

#include <assert.h>
#include <raylib.h>

// --- Constants ---

static const Vector2 PLAYER_START_POS = {101, 176};

// --- Global state ---

static Vector2 g_playerPos;

// --- Player functions ---

void game__playerInit(void) {
  g_playerPos = PLAYER_START_POS;
}

void game__playerUpdate(void) {}

Vector2 game__playerGetPos(void) {
  return g_playerPos;
}
