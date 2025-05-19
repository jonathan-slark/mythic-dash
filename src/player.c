#include "player.h"

#include <assert.h>
#include <raylib.h>

// --- Constants ---

static const Vector2 PLAYER_START_POS = { 101, 176 };

// --- Global state ---

static Vector2 gPlayerPos;

// --- Player functions ---

void player_init(void) {
  gPlayerPos = PLAYER_START_POS;
}

void player_update(void) {}

Vector2 player_getPos(void) {
  return gPlayerPos;
}
