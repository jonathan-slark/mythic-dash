#include "player.h"
#include <assert.h>
#include <engine/engine.h>
#include <raylib.h>
#include <raymath.h>
#include "../actor/actor.h"
#include "../audio/audio.h"
#include "../creature/creature.h"
#include "../debug/debug.h"
#include "../maze/maze.h"
#include "log/log.h"

// --- Types ---

typedef struct Player {
  game_Actor*      actor;
  game_PlayerState state;
  int              lives;
  int              score;
  int              coinsCollected;
  float            swordTimer;
  float            deadTimer;
  int              scoreMultiplier;
  float            coinSlowTimer;
  float            swordSlowTimer;
  int              lastScoreBonusLife;
} Player;

// --- Constants ---

static const Vector2     PLAYER_START_POS    = { 14 * TILE_SIZE, 10 * TILE_SIZE };
static const float       PLAYER_MAX_SPEED    = 80.0f;
static const game_Dir    PLAYER_START_DIR    = DIR_LEFT;
static const KeyboardKey PLAYER_KEYS[]       = { KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT };
static const int         SCORE_COIN          = 10;
static const int         SCORE_SWORD         = 50;
static const float       PLAYER_DEAD_TIMER   = 2.0f;
static const int         creature_BASE_SCORE = 200;
static const float       COIN_SLOW_TIMER     = 0.2f;
static const float       SWORD_SLOW_TIMER    = 0.2f;
static const float       PLAYER_SLOW_SPEED   = 72.0f;  // 10% slow
static const int         MAX_LEVEL           = 20;
static const float       SWORD_MAX_TIMER     = 4.0f;
static const float       SWORD_MIN_TIMER     = 2.4f;   // 60%
static const int         SCORE_EXTRA_LIFE    = 10000;
static const int         SCORE_CHEST         = 100;

// --- Global state ---

static Player g_player = {
  .actor              = nullptr,
  .state              = PLAYER_NORMAL,
  .lives              = PLAYER_LIVES,
  .score              = 0,
  .coinsCollected     = 0,
  .swordTimer         = 0.0f,
  .deadTimer          = 0.0f,
  .scoreMultiplier    = 1,
  .lastScoreBonusLife = 0,
};

// --- Helper functions ---

static void playerCoinPickup(void) {
  g_player.coinSlowTimer  = COIN_SLOW_TIMER;
  g_player.score         += SCORE_COIN;
  g_player.coinsCollected++;
  actor_setSpeed(g_player.actor, PLAYER_SLOW_SPEED);
  audo_playChime(player_getPos());
}

static int getChestScoreMultiplier(void) {
  int   level = game_getLevel();
  float scale = (level - 1) / 19.0f;
  return 1 + (int) (scale * 49.0f);
}

static int getChestScore(void) { return getChestScoreMultiplier() * SCORE_CHEST; }

static void playerChestPickup(void) {
  g_player.score += getChestScore();
  audio_playPickup(player_getPos());
}

static void playerKeyPickup(void) { audio_playPickup(player_getPos()); }

static float getSwordTimer(void) {
  float t = fminf(fmaxf((game_getLevel() - 1) / (MAX_LEVEL - 1.0f), 0.0f), 1.0f);
  t       = 1.0f - powf(1.0f - t, 2.0f);  // ease-out curve
  return SWORD_MAX_TIMER - t * (SWORD_MAX_TIMER - SWORD_MIN_TIMER);
}

static void playerSwordPickup(void) {
  g_player.state           = PLAYER_SWORD;
  g_player.swordTimer      = getSwordTimer();
  g_player.swordSlowTimer  = SWORD_SLOW_TIMER;
  g_player.score          += SCORE_SWORD;
  g_player.coinsCollected++;  // Counts as a coin in terms on level being cleared
  g_player.scoreMultiplier = 1;
  actor_setSpeed(g_player.actor, PLAYER_SLOW_SPEED);
  audio_resetChimePitch();
}

static void playerCoinSlowUpdate(float frameTime) {
  assert(frameTime >= 0.0f);

  if (g_player.coinSlowTimer == 0.0f) return;
  g_player.coinSlowTimer = fmaxf(g_player.coinSlowTimer -= frameTime, 0.0f);
  if (g_player.coinSlowTimer == 0.0f && g_player.swordSlowTimer == 0.0f)
    actor_setSpeed(g_player.actor, PLAYER_MAX_SPEED);
}

static void playerSwordSlowUpdate(float frameTime) {
  assert(frameTime >= 0.0f);

  if (g_player.swordSlowTimer == 0.0f) return;
  g_player.swordSlowTimer = fmaxf(g_player.swordSlowTimer -= frameTime, 0.0f);
  if (g_player.swordSlowTimer == 0.0f && g_player.coinSlowTimer == 0.0f)
    actor_setSpeed(g_player.actor, PLAYER_MAX_SPEED);
}

static void playerSwordUpdate(float frameTime) {
  assert(frameTime >= 0.0f);

  if (g_player.swordTimer == 0.0f) return;

  g_player.swordTimer = fmaxf(g_player.swordTimer -= frameTime, 0.0f);
  if (g_player.swordTimer == 0.0f) {
    g_player.state = PLAYER_NORMAL;
    creature_swordDrop();
  }
}

static void playerCheckPickups(void) {
  // Check centre of tile, feels right.
  Vector2 pos = actor_getPos(g_player.actor);
  pos         = Vector2AddValue(pos, ACTOR_SIZE / 2.0f);
  if (maze_isCoin(pos)) {
    maze_pickupCoin(pos);
    playerCoinPickup();
  } else if (maze_isSword(pos)) {
    maze_pickupSword(pos);
    playerSwordPickup();
    creature_swordPickup();
  } else if (maze_isChest(pos)) {
    maze_pickupChest(pos, getChestScore());
    playerChestPickup();
  } else if (maze_isKey(pos)) {
    maze_pickupKey(pos);
    playerKeyPickup();
  }

  if (maze_getCoinCount() == g_player.coinsCollected) {
    game_nextLevel();
  }
}

static bool playerCheckTraps(void) {
  Vector2 pos = actor_getPos(g_player.actor);
  pos         = Vector2AddValue(pos, ACTOR_SIZE / 2.0f);
  if (maze_isTrap(pos)) {
    maze_trapTriggered(pos);
    player_dead();
    return true;
  }
  return false;
}

static void playerCheckScore() {
  if (g_player.lives < PLAYER_MAX_LIVES && g_player.score > g_player.lastScoreBonusLife &&
      (g_player.score % SCORE_EXTRA_LIFE == 0)) {
    g_player.lives              += 1;
    g_player.lastScoreBonusLife  = g_player.score;
    LOG_DEBUG(game_log, "Player gained bonus life at score:", g_player.score);
  }
}

// --- Player functions ---

bool player_init(void) {
  assert(g_player.actor == nullptr);
  g_player.actor = actor_create(
      PLAYER_START_POS, (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, PLAYER_START_DIR, PLAYER_MAX_SPEED, true
  );
  return g_player.actor != nullptr;
}

void player_restart(void) {
  assert(g_player.actor != nullptr);
  g_player.state           = PLAYER_NORMAL;
  g_player.swordTimer      = 0.0f;
  g_player.scoreMultiplier = 1;
  actor_setPos(g_player.actor, PLAYER_START_POS);
  actor_setDir(g_player.actor, PLAYER_START_DIR);
  actor_startMoving(g_player.actor);
  audio_resetChimePitch();
}

void player_reset() {
  player_restart();
  g_player.coinsCollected = 0;
}

void player_totalReset() {
  player_reset();
  g_player.lives              = PLAYER_LIVES;
  g_player.score              = 0;
  g_player.lastScoreBonusLife = 0;
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
  if (engine_isKeyPressed(KEY_G)) debug_toggleCreatureOverlay();
  if (engine_isKeyPressed(KEY_I)) debug_togglePlayerImmune();
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

  game_Dir dir = DIR_NONE;
  for (int i = 0; i < DIR_COUNT; i++) {
    if (engine_isKeyDown(PLAYER_KEYS[i]) && actor_canMove(g_player.actor, (game_Dir) i, slop)) {
      dir = (game_Dir) i;
      break;
    }
  }
  // Player keeps continually moving till they hit a wall
  if (dir == DIR_NONE) dir = actor_getDir(g_player.actor);

  actor_move(g_player.actor, dir, frameTime);

  if (playerCheckTraps()) return;  // Dead!
  playerCheckPickups();
  playerCheckScore();
}

Vector2 player_getPos(void) {
  assert(g_player.actor != nullptr);
  return actor_getPos(g_player.actor);
}

game_Dir player_getDir(void) {
  assert(g_player.actor != nullptr);
  return actor_getDir(g_player.actor);
}

int player_getScore(void) {
  assert(g_player.actor != nullptr);
  return g_player.score;
}

game_PlayerState player_getState(void) {
  assert(g_player.state >= 0 && g_player.state < PLAYER_STATE_COUNT);
  return g_player.state;
}

bool player_isMoving(void) {
  assert(g_player.actor != nullptr);
  return actor_isMoving(g_player.actor);
}

game_Actor* player_getActor(void) {
  assert(g_player.actor != nullptr);
  return g_player.actor;
}

game_Tile player_tileAhead(int tileNum) {
  assert(tileNum > 0);

  Vector2 pos = player_getPos();
  Vector2AddValue(pos, ACTOR_SIZE / 2.0f);
  game_Tile tile = maze_getTile(pos);
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

float player_getMaxSpeed(void) { return PLAYER_MAX_SPEED; }

int player_getLives(void) {
  assert(g_player.lives >= 0 && g_player.lives < PLAYER_MAX_LIVES);
  return g_player.lives;
}

void player_dead(void) {
  if (debug_isPlayerImmune()) return;

  g_player.lives     -= 1;
  g_player.state      = PLAYER_DEAD;
  g_player.deadTimer  = PLAYER_DEAD_TIMER;
  audio_resetChimePitch();
  audio_playDeath(player_getPos());
}

bool player_hasSword(void) {
  assert(g_player.swordTimer >= 0.0f);
  return g_player.swordTimer > 0.0f;
}

float player_getSwordTimer(void) {
  assert(g_player.swordTimer >= 0.0f);
  return g_player.swordTimer;
}

void player_killedCreature(int creatureID) {
  assert(creatureID >= 0 && creatureID < CREATURE_COUNT);

  int score                 = creature_BASE_SCORE * g_player.scoreMultiplier;
  g_player.score           += score;
  g_player.scoreMultiplier *= 2;
  creature_setScore(creatureID, score);
}

int player_getCoinsCollected(void) {
  assert(g_player.coinsCollected >= 0);
  return g_player.coinsCollected;
}

int player_getNextExtraLifeScore(void) { return (g_player.score / SCORE_EXTRA_LIFE + 1) * SCORE_EXTRA_LIFE; }
