#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include "../engine/engine.h"
#include "game_internal.h"

// --- Constants ---

static const Vector2 PLAYER_START_POS = {101, 176};
static const float   PLAYER_SPEED     = 100.0f;

// --- Global state ---

static Actor g_player;

// --- Player functions ---

void game__playerInit(void) {
  g_player.pos = PLAYER_START_POS;
  g_player.dir = Left;
}

void game__playerUpdate(float frameTime) {
  float distance = PLAYER_SPEED * frameTime;
  if (distance <= 0.0f) {
    return;
  }

  Dir dir = None;
  if (engine_isKeyDown(KEY_LEFT) && game__actorCanMove(g_player, Left, distance)) dir = Left;
  if (engine_isKeyDown(KEY_RIGHT) && game__actorCanMove(g_player, Right, distance)) dir = Right;
  if (engine_isKeyDown(KEY_UP) && game__actorCanMove(g_player, Up, distance)) dir = Up;
  if (engine_isKeyDown(KEY_DOWN) && game__actorCanMove(g_player, Down, distance)) dir = Down;

  if (dir == None) {
    dir = g_player.dir;
    if (!game__actorCanMove(g_player, dir, distance)) return;
  } else {
    g_player.dir = dir;
  }

  g_player.pos = Vector2Add(g_player.pos, Vector2Scale(VELS[dir], distance));
  LOG_TRACE(game__log, "Player moved to %f, %f", g_player.pos.x, g_player.pos.y);
}

Vector2 game__playerGetPos(void) { return g_player.pos; }
