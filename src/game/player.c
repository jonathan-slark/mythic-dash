#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include "../engine/engine.h"
#include "game_internal.h"

// --- Constants ---

static const Vector2 PLAYER_START_POS = {101, 176};
static const float PLAYER_SPEED       = 100.0f;

// --- Global state ---

static Actor g_player;

// --- Player functions ---

void game__playerInit(void) { g_player.pos = PLAYER_START_POS; }

void game__playerUpdate(float frameTime) {
  g_player.dir = None;
  if (engine_isKeyDown(KEY_LEFT))
    g_player.dir = Left;
  if (engine_isKeyDown(KEY_RIGHT))
    g_player.dir = Right;
  if (engine_isKeyDown(KEY_UP))
    g_player.dir = Up;
  if (engine_isKeyDown(KEY_DOWN))
    g_player.dir = Down;

  if (g_player.dir != None) {
    Vector2 vel    = Vector2Normalize(VELS[g_player.dir]);
    float distance = PLAYER_SPEED * frameTime;
    if (distance > 0.0f && game__actorCanMove(g_player, distance)) {
      g_player.pos = Vector2Add(g_player.pos, Vector2Scale(vel, distance));
    }
  }
}

Vector2 game__playerGetPos(void) { return g_player.pos; }
