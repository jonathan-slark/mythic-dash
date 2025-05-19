#include "game.h"

#include <assert.h>
#include <raylib.h>

// --- Constants ---

static const Vector2 PLAYER_START_POS = { 101, 176 };

// --- Global state ---

static Vector2 gPlayerPos;

// --- Player functions ---

void game_playerInit(void) {
  gPlayerPos = PLAYER_START_POS;
}

void game_playerUpdate(void) {}

Vector2 game_playerGetPos(void) {
  return gPlayerPos;
}
