#include <assert.h>
#include <engine/engine.h>
#include <raylib.h>
#include <raymath.h>
#include "game.h"

// --- Constants ---

static const Vector2     PLAYER_START_POS = { 14 * TILE_SIZE, 13 * TILE_SIZE };
static const float       PLAYER_SPEED     = 60.0f;
static const game__Dir   PLAYER_START_DIR = DIR_LEFT;
static const KeyboardKey PLAYER_KEYS[]    = { KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT };

// --- Global state ---

static game__Actor* g_player = nullptr;

// --- Player functions ---

bool player_init(void) {
  assert(g_player == nullptr);
  g_player = actor_create(PLAYER_START_POS, (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, PLAYER_START_DIR, PLAYER_SPEED);
  return g_player != nullptr;
}

void player_shutdown(void) {
  assert(g_player != nullptr);
  actor_destroy(&g_player);
  assert(g_player == nullptr);
}

void player_update(float frameTime, float slop) {
  assert(g_player != nullptr);

#ifndef NDEBUG
  if (engine_isKeyPressed(KEY_F)) debug_toggleFPSOverlay();
  if (engine_isKeyPressed(KEY_M)) debug_toggleMazeOverlay();
  if (engine_isKeyPressed(KEY_P)) debug_togglePlayerOverlay();
  if (engine_isKeyPressed(KEY_G)) debug_toggleGhostOverlay();
#endif

  game__Dir dir = DIR_NONE;
  for (int i = 0; i < DIR_COUNT; i++) {
    if (engine_isKeyDown(PLAYER_KEYS[i]) && actor_canMove(g_player, (game__Dir) i, slop)) {
      dir = (game__Dir) i;
      break;
    }
  }
  // Player keeps continually moving till they hit a wall
  if (dir == DIR_NONE) dir = actor_getDir(g_player);

  actor_move(g_player, dir, frameTime);
}

Vector2 player_getPos(void) {
  assert(g_player != nullptr);
  return actor_getPos(g_player);
}

game__Dir player_getDir(void) {
  assert(g_player != nullptr);
  return actor_getDir(g_player);
}

bool player_isMoving(void) {
  assert(g_player != nullptr);
  return actor_isMoving(g_player);
}

game__Actor* player_getActor(void) {
  assert(g_player != nullptr);
  return g_player;
}

game__Tile player_tileAhead(int tileNum) {
  game__Tile tile = maze_getTile(player_getPos());
  switch (player_getDir()) {
    case DIR_UP:
      tile.row -= tileNum;
      if (tile.row < 0) tile.row = 0;
      break;
    case DIR_RIGHT:
      tile.col += tileNum;
      int cols  = maze_getCols();
      if (tile.col >= cols) tile.col = cols - 1;
      break;
    case DIR_DOWN:
      tile.row += tileNum;
      int rows  = maze_getRows();
      if (tile.row >= rows) tile.row = rows - 1;
      break;
    case DIR_LEFT:
      tile.col -= tileNum;
      if (tile.row < 0) tile.row = 0;
      break;
    default: assert(false);
  }
  return tile;
}
