#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include "../engine/engine.h"
#include "game_internal.h"

// --- Constants ---

static const Vector2 PLAYER_START_POS = {109.0f, 184.0f};
static const float   PLAYER_SPEED     = 100.0f;

// --- Global state ---

static Actor g_player;

// --- Player functions ---

void game__playerInit(void) {
  g_player.pos = PLAYER_START_POS;
  g_player.dir = Left;
}

void game__playerUpdate(float frameTime) {
  Actor moved = g_player;

  if (engine_isKeyDown(KEY_UP)) {
    moved.dir = Up;
  } else if (engine_isKeyDown(KEY_RIGHT)) {
    moved.dir = Right;
  } else if (engine_isKeyDown(KEY_DOWN)) {
    moved.dir = Down;
  } else if (engine_isKeyDown(KEY_LEFT)) {
    moved.dir = Left;
  }

  // Attempt to move the player
  moved.pos = Vector2Add(g_player.pos, Vector2Scale(VELS[moved.dir], frameTime * PLAYER_SPEED));
  if (game__isHittingWall(game__getActorAABB(moved))) {
    // Attempt to continue moving the player in the same direction
    moved     = g_player;
    moved.pos = Vector2Add(g_player.pos, Vector2Scale(VELS[moved.dir], frameTime * PLAYER_SPEED));
    if (game__isHittingWall(game__getActorAABB(moved))) {
      return;
    } else {
      g_player = moved;
      LOG_TRACE(game__log, "Moved player to %f, %f", g_player.pos.x, g_player.pos.y);
    }
  } else {
    g_player = moved;
    LOG_TRACE(game__log, "Moved player to %f, %f", g_player.pos.x, g_player.pos.y);
  }
}

Vector2 game__playerGetPos(void) { return g_player.pos; }

#ifndef NDEBUG
void game__playerOverlay(void) {
  Actor adjusted = g_player;
  adjusted.pos   = POS_ADJUST(adjusted.pos);
  AABB aabb      = game__getActorAABB(adjusted);
  engine_drawRectangleOutline((Rectangle) {aabb.min.x, aabb.min.y, aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y},
                              RED);
}
#endif
