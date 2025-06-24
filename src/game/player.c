#include <assert.h>
#include <engine/engine.h>
#include <raylib.h>
#include <raymath.h>
#include "game.h"

// --- Types ---

typedef struct Player {
  game__Actor*      actor;
  game__PlayerState state;
  int               lives;
  int               score;
  int               coinsCollected;
  float             swordTimer;
  float             deadTimer;
  int               scoreMultiplier;
  float             scoreMultiplierTimer;
  float             coinSlowTimer;
  float             swordSlowTimer;
} Player;

// --- Constants ---

static const Vector2     PLAYER_START_POS       = { 14 * TILE_SIZE, 13 * TILE_SIZE };
static const float       PLAYER_SPEED           = 60.0f;
static const game__Dir   PLAYER_START_DIR       = DIR_LEFT;
static const KeyboardKey PLAYER_KEYS[]          = { KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT };
static const int         SCORE_COIN             = 10;
static const int         SCORE_SWORD            = 50;
static const float       SWORD_TIMER            = 6.0f;  // Sword animation lasts 400ms
static const float       PLAYER_DEAD_TIMER      = 2.0f;
static const int         GHOST_BASE_SCORE       = 200;
static const float       SCORE_MULTIPLIER_TIMER = 5.0f;
static const float       COIN_SLOW_TIMER        = 0.5f;
static const float       SWORD_SLOW_TIMER       = 0.5f;
static const float       PLAYER_SLOW_SPED       = 50.f;

// --- Global state ---

static Player g_player = {
  .actor                = nullptr,
  .state                = PLAYER_NORMAL,
  .lives                = PLAYER_LIVES,
  .score                = 0,
  .coinsCollected       = 0,
  .swordTimer           = 0.0f,
  .deadTimer            = 0.0f,
  .scoreMultiplier      = 1,
  .scoreMultiplierTimer = 0.0f
};

// --- Helper functions ---

static void playerCoinPickup(void) {
  g_player.coinSlowTimer  = COIN_SLOW_TIMER;
  g_player.score         += SCORE_COIN;
  g_player.coinsCollected++;
  actor_setSpeed(g_player.actor, PLAYER_SLOW_SPED);
}

static void playerSwordPickup(void) {
  g_player.state           = PLAYER_SWORD;
  g_player.swordTimer      = SWORD_TIMER;
  g_player.swordSlowTimer  = SWORD_SLOW_TIMER;
  g_player.score          += SCORE_SWORD;
  actor_setSpeed(g_player.actor, PLAYER_SLOW_SPED);
}

static void playerCoinSlowUpdate(float frameTime) {
  if (g_player.coinSlowTimer == 0.0f) return;
  g_player.coinSlowTimer = fmaxf(g_player.coinSlowTimer -= frameTime, 0.0f);
  if (g_player.coinSlowTimer == 0.0f && g_player.swordSlowTimer == 0.0f) actor_setSpeed(g_player.actor, PLAYER_SPEED);
}

static void playerSwordSlowUpdate(float frameTime) {
  if (g_player.swordSlowTimer == 0.0f) return;
  g_player.swordSlowTimer = fmaxf(g_player.swordSlowTimer -= frameTime, 0.0f);
  if (g_player.swordSlowTimer == 0.0f && g_player.coinSlowTimer == 0.0f) actor_setSpeed(g_player.actor, PLAYER_SPEED);
}

static void playerSwordUpdate(float frameTime) {
  if (g_player.swordTimer == 0.0f) return;

  g_player.swordTimer = fmaxf(g_player.swordTimer -= frameTime, 0.0f);
  if (g_player.swordTimer == 0.0f) {
    g_player.state = PLAYER_NORMAL;
    ghost_swordDrop();
  }
}

static void playerScoreMultiplierUpdate(float frameTime) {
  if (g_player.scoreMultiplierTimer == 0.0f) return;

  g_player.scoreMultiplierTimer = fmaxf(g_player.scoreMultiplierTimer -= frameTime, 0.0f);
  if (g_player.scoreMultiplierTimer == 0.0f) {
    g_player.scoreMultiplier = 1;
  }
}

static void playerCheckPickups(void) {
  // Check centre of tile, feels right.
  Vector2 pos = actor_getPos(g_player.actor);
  pos         = Vector2AddValue(pos, ACTOR_SIZE / 2.0f);
  if (maze_isCoin(pos)) {
    maze_pickupCoin(pos);
    playerCoinPickup();
    if (maze_getCoinCount() == g_player.coinsCollected) {
      game_nextLevel();
    }
  }
  if (maze_isSword(pos)) {
    maze_pickupSword(pos);
    playerSwordPickup();
    ghost_swordPickup();
  }
}

// --- Player functions ---

bool player_init(void) {
  assert(g_player.actor == nullptr);
  g_player.actor = actor_create(PLAYER_START_POS, (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, PLAYER_START_DIR, PLAYER_SPEED);
  return g_player.actor != nullptr;
}

void player_restart(void) {
  assert(g_player.actor != nullptr);
  g_player.state                = PLAYER_NORMAL;
  g_player.swordTimer           = 0.0f;
  g_player.scoreMultiplier      = 1;
  g_player.scoreMultiplierTimer = 0.0f;
  actor_setPos(g_player.actor, PLAYER_START_POS);
  actor_setDir(g_player.actor, PLAYER_START_DIR);
  actor_startMoving(g_player.actor);
}

void player_reset() {
  player_restart();
  g_player.coinsCollected = 0;
}

void player_totalReset() {
  player_reset();
  g_player.lives = PLAYER_LIVES;
  g_player.score = 0;
}

void player_shutdown(void) {
  assert(g_player.actor != nullptr);
  actor_destroy(&g_player.actor);
  assert(g_player.actor == nullptr);
}

void player_update(float frameTime, float slop) {
  assert(g_player.actor != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= 0.0f);

#ifndef NDEBUG
  if (engine_isKeyPressed(KEY_F)) debug_toggleFPSOverlay();
  if (engine_isKeyPressed(KEY_M)) debug_toggleMazeOverlay();
  if (engine_isKeyPressed(KEY_P)) debug_togglePlayerOverlay();
  if (engine_isKeyPressed(KEY_G)) debug_toggleGhostOverlay();
#endif

  if (g_player.state == PLAYER_DEAD) {
    g_player.deadTimer = fmaxf(g_player.deadTimer - frameTime, 0.0f);
    if (g_player.deadTimer == 0.0f) {
      if (g_player.lives == 0) {
        game_over();
      } else {
        game_playerDead();
      }
    }
    return;
  }

  playerCoinSlowUpdate(frameTime);
  playerSwordUpdate(frameTime);
  playerSwordSlowUpdate(frameTime);
  playerScoreMultiplierUpdate(frameTime);

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

  playerCheckPickups();
}

Vector2 player_getPos(void) {
  assert(g_player.actor != nullptr);
  return actor_getPos(g_player.actor);
}

game__Dir player_getDir(void) {
  assert(g_player.actor != nullptr);
  return actor_getDir(g_player.actor);
}

int player_getScore(void) {
  assert(g_player.actor != nullptr);
  return g_player.score;
}

game__PlayerState player_getState(void) { return g_player.state; }

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
  g_player.lives     -= 1;
  g_player.state      = PLAYER_DEAD;
  g_player.deadTimer  = PLAYER_DEAD_TIMER;
}

bool player_hasSword(void) { return g_player.swordTimer > 0.0f; }

float player_getSwordTimer(void) { return g_player.swordTimer; }

void player_killedGhost(int ghostID) {
  int score                      = GHOST_BASE_SCORE * g_player.scoreMultiplier;
  g_player.score                += score;
  g_player.scoreMultiplier      *= 2;
  g_player.scoreMultiplierTimer  = SCORE_MULTIPLIER_TIMER;
  ghost_setScore(ghostID, score);
}
