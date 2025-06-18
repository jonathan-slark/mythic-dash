#include <assert.h>
#include <engine/engine.h>
#include <raylib.h>
#include <raymath.h>
#include "game.h"

// --- Types ---

typedef struct Player {
  game__Actor* actor;
  int          lives;
} Player;

// --- Constants ---

static const Vector2     PLAYER_START_POS = { 14 * TILE_SIZE, 13 * TILE_SIZE };
static const float       PLAYER_SPEED     = 60.0f;
static const game__Dir   PLAYER_START_DIR = DIR_LEFT;
static const KeyboardKey PLAYER_KEYS[]    = { KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT };

// --- Global state ---

static Player g_player = { .actor = nullptr, .lives = PLAYER_LIVES };

// --- Helper functions ---

void playerRestart(void) {
  assert(g_player.actor != nullptr);
  actor_setPos(g_player.actor, PLAYER_START_POS);
  actor_setDir(g_player.actor, PLAYER_START_DIR);
}

// --- Player functions ---

bool player_init(void) {
  assert(g_player.actor == nullptr);
  g_player.actor = actor_create(PLAYER_START_POS, (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, PLAYER_START_DIR, PLAYER_SPEED);
  return g_player.actor != nullptr;
}

void player_reset() {
  playerRestart();
  g_player.lives = PLAYER_LIVES;
}

void player_shutdown(void) {
  assert(g_player.actor != nullptr);
  actor_destroy(&g_player.actor);
  assert(g_player.actor == nullptr);
}

void player_update(float frameTime, float slop) {
  assert(g_player.actor != nullptr);

#ifndef NDEBUG
  if (engine_isKeyPressed(KEY_F)) debug_toggleFPSOverlay();
  if (engine_isKeyPressed(KEY_M)) debug_toggleMazeOverlay();
  if (engine_isKeyPressed(KEY_P)) debug_togglePlayerOverlay();
  if (engine_isKeyPressed(KEY_G)) debug_toggleGhostOverlay();
#endif

  game__Dir dir = DIR_NONE;
  for (int i = 0; i < DIR_COUNT; i++) {
    if (engine_isKeyDown(PLAYER_KEYS[i]) && actor_canMove(g_player.actor, (game__Dir) i, slop)) {
      dir = (game__Dir) i;
      break;
    }
  }
  // Player keeps continually moving till they hit a wall
  if (dir == DIR_NONE) dir = actor_getDir(g_player.actor);

  actor_move(g_player.actor, dir, frameTime);
}

Vector2 player_getPos(void) {
  assert(g_player.actor != nullptr);
  return actor_getPos(g_player.actor);
}

game__Dir player_getDir(void) {
  assert(g_player.actor != nullptr);
  return actor_getDir(g_player.actor);
}

bool player_isMoving(void) {
  assert(g_player.actor != nullptr);
  return actor_isMoving(g_player.actor);
}

game__Actor* player_getActor(void) {
  assert(g_player.actor != nullptr);
  return g_player.actor;
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
      if (tile.col < 0) tile.col = 0;
      break;
    default: assert(false);
  }
  return tile;
}

float player_getSpeed(void) { return PLAYER_SPEED; }

int player_getLives(void) { return g_player.lives; }

void player_dead(void) {
  g_player.lives -= 1;
  if (g_player.lives == 0) {
    game_over();
  } else {
    playerRestart();
  }
}
