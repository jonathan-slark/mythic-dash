#include <assert.h>
#include <raylib.h>
#include "../engine/engine.h"
#include "game_internal.h"

// --- Constants ---

static const Vector2 PLAYER_START_POS = {101, 176};
static const float PLAYER_SPEED       = 100.0f;

// --- Global state ---

static Vector2 g_playerPos;

// --- Player functions ---

void game__playerInit(void) { g_playerPos = PLAYER_START_POS; }

void game__playerUpdate(float frameTime) {
  if (engine_isKeyDown(KEY_LEFT)) {
    g_playerPos.x -= frameTime * PLAYER_SPEED;
  }
  if (engine_isKeyDown(KEY_RIGHT)) {
    g_playerPos.x += frameTime * PLAYER_SPEED;
  }
  if (engine_isKeyDown(KEY_UP)) {
    g_playerPos.y -= frameTime * PLAYER_SPEED;
  }
  if (engine_isKeyDown(KEY_DOWN)) {
    g_playerPos.y += frameTime * PLAYER_SPEED;
  }
}

Vector2 game__playerGetPos(void) { return g_playerPos; }
