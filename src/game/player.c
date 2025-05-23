#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include "../engine/engine.h"
#include "internal.h"

// --- Constants ---

static const Vector2 PLAYER_START_POS = {109.0f, 184.0f};
static const float   PLAYER_SPEED     = 100.0f;

// --- Global state ---

static Actor* g_player;

// --- Player functions ---

bool player_init(void) {
  assert(g_player == nullptr);
  g_player = actor_create(PLAYER_START_POS, (Vector2) {ACTOR_SIZE, ACTOR_SIZE}, Left, PLAYER_SPEED);
  return g_player != nullptr;
}

void player_shutdown(void) {
  assert(g_player != nullptr);
  actor_destroy(&g_player);
}

void player_update(float frameTime) {
  assert(g_player != nullptr);

#ifndef NDEBUG
  if (engine_isKeyPressed(KEY_O)) {
    game__isOverlayEnabled = !game__isOverlayEnabled;
  }
#endif

  Dir dir = actor_getDir(g_player);
  if (engine_isKeyDown(KEY_UP)) {
    dir = Up;
  } else if (engine_isKeyDown(KEY_RIGHT)) {
    dir = Right;
  } else if (engine_isKeyDown(KEY_DOWN)) {
    dir = Down;
  } else if (engine_isKeyDown(KEY_LEFT)) {
    dir = Left;
  }

  actor_move(g_player, dir, frameTime);
  actor_checkMazeCollision(g_player);
}

Vector2 player_getPos(void) {
  assert(g_player != nullptr);
  return actor_getPos(g_player);
}

#ifndef NDEBUG
void player_overlay(void) {
  assert(g_player != nullptr);
  Vector2 pos  = POS_ADJUST(player_getPos());
  AABB    aabb = actor_getAABB(g_player);
  engine_drawRectangleOutline((Rectangle) {pos.x, pos.y, aabb.max.x - aabb.min.x, aabb.max.y - aabb.min.y},
                              OVERLAY_COLOUR);
}
#endif
