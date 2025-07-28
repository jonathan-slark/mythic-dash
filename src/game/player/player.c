#include "player.h"
#include <assert.h>
#include <engine/engine.h>
#include <raylib.h>
#include <raymath.h>
#include "../actor/actor.h"
#include "../audio/audio.h"
#include "../creature/creature.h"
#include "../debug/debug.h"
#include "../input/input.h"
#include "../maze/maze.h"
#include "../scores/scores.h"
#include "game/game.h"
#include "log/log.h"

// --- Types ---

typedef struct Player {
  game_Actor*      actor;
  game_PlayerState state;
  int              lives;
  int              score;
  int              previousScore;
  double           time;
  double           previousTime;
  int              coinsCollected;
  float            swordTimer;
  float            deadTimer;
  int              scoreMultiplier;
  float            coinSlowTimer;
  float            swordSlowTimer;
  int              lastScoreBonusLife;
  player_levelData levelData[LEVEL_COUNT];
  player_levelData fullRun;
} Player;

// --- Constants ---

static const Vector2     PLAYER_START_POS                    = { 14 * TILE_SIZE, 10 * TILE_SIZE };
static const float       PLAYER_MAX_SPEED[DIFFICULTY_COUNT]  = { 88.0f, 80.0f, 80.0f };
static const game_Dir    PLAYER_START_DIR                    = DIR_LEFT;
static const KeyboardKey PLAYER_KEYS[]                       = { KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT };
static const int         SCORE_COIN                          = 10;
static const int         SCORE_SWORD                         = 50;
static const float       PLAYER_DEAD_TIMER                   = 2.0f;
static const int         creature_BASE_SCORE                 = 200;
static const float       COIN_SLOW_TIMER                     = 0.2f;
static const float       SWORD_SLOW_TIMER                    = 0.2f;
static const float       PLAYER_SLOW_SPEED[DIFFICULTY_COUNT] = { 79.2f, 72.0f, 72.0f };  // 10% slow
static const int         MAX_LEVEL                           = 7;
static const float       SWORD_MAX_TIMER[DIFFICULTY_COUNT]   = { 14.0f, 10.0f, 6.0f };
static const float       SWORD_MIN_TIMER[DIFFICULTY_COUNT]   = { 8.4f, 6.0f, 3.6f };     // 60%
static const int         SCORE_EXTRA_LIFE[DIFFICULTY_COUNT]  = { 3000, 5000, 10000 };
static const int         SCORE_CHEST                         = 100;

// --- Global state ---

static Player g_player = {
  .actor              = nullptr,
  .state              = PLAYER_NORMAL,
  .lives              = PLAYER_LIVES,
  .score              = 0,
  .previousScore      = 0,
  .coinsCollected     = 0,
  .swordTimer         = 0.0f,
  .deadTimer          = 0.0f,
  .scoreMultiplier    = 1,
  .coinSlowTimer      = 0.0f,
  .swordSlowTimer     = 0.0f,
  .lastScoreBonusLife = 0,
  .levelData          = {}
};

// --- Helper functions ---

static void coinPickup(void) {
  g_player.coinSlowTimer  = COIN_SLOW_TIMER;
  g_player.score         += SCORE_COIN;
  g_player.coinsCollected++;
  actor_setSpeed(g_player.actor, PLAYER_SLOW_SPEED[game_getDifficulty()]);
  audo_playChime(player_getPos());
}

static int getChestScoreMultiplier(void) {
  int   level = game_getLevel();
  float scale = level / 19.0f;
  return 1 + (int) (scale * 49.0f);
}

static int getChestScore(void) { return getChestScoreMultiplier() * SCORE_CHEST; }

static void chestPickup(void) {
  g_player.score += getChestScore();
  audio_playPickup(player_getPos());
}

static void keyPickup(void) { audio_playPickup(player_getPos()); }

static float getSwordTimer(void) {
  float t                    = fminf(fmaxf((game_getLevel() - 1) / (MAX_LEVEL - 1.0f), 0.0f), 1.0f);
  t                          = 1.0f - powf(1.0f - t, 2.0f);  // ease-out curve
  game_Difficulty difficulty = game_getDifficulty();
  return SWORD_MAX_TIMER[difficulty] - t * (SWORD_MAX_TIMER[difficulty] - SWORD_MIN_TIMER[difficulty]);
}

static void swordPickup(void) {
  g_player.state           = PLAYER_SWORD;
  g_player.swordTimer      = getSwordTimer();
  g_player.swordSlowTimer  = SWORD_SLOW_TIMER;
  g_player.score          += SCORE_SWORD;
  g_player.coinsCollected++;  // Counts as a coin in terms on level being cleared
  g_player.scoreMultiplier = 1;
  actor_setSpeed(g_player.actor, PLAYER_SLOW_SPEED[game_getDifficulty()]);
  audio_resetChimePitch();
}

static void coinSlowUpdate(double frameTime) {
  assert(frameTime >= 0.0f);

  if (g_player.coinSlowTimer == 0.0f) return;
  g_player.coinSlowTimer = fmaxf(g_player.coinSlowTimer -= frameTime, 0.0f);
  if (g_player.coinSlowTimer == 0.0f && g_player.swordSlowTimer == 0.0f)
    actor_setSpeed(g_player.actor, PLAYER_MAX_SPEED[game_getDifficulty()]);
}

static void swordSlowUpdate(double frameTime) {
  assert(frameTime >= 0.0f);

  if (g_player.swordSlowTimer == 0.0f) return;
  g_player.swordSlowTimer = fmaxf(g_player.swordSlowTimer -= frameTime, 0.0f);
  if (g_player.swordSlowTimer == 0.0f && g_player.coinSlowTimer == 0.0f)
    actor_setSpeed(g_player.actor, PLAYER_MAX_SPEED[game_getDifficulty()]);
}

static void swordUpdate(double frameTime) {
  assert(frameTime >= 0.0f);

  if (g_player.swordTimer == 0.0f) return;

  g_player.swordTimer = fmaxf(g_player.swordTimer -= frameTime, 0.0f);
  if (g_player.swordTimer == 0.0f) {
    g_player.state = PLAYER_NORMAL;
    creature_swordDrop();
  }
}

static void levelClear(void) {
  g_player.time += engine_getTime() - g_player.previousTime;

  int level = game_getLevel();

  if (game_getDifficulty() == DIFFICULTY_ARCADE) {
    // Araced Mode is 100% deterministic, game updates using fixed timestep
    g_player.levelData[level].time = g_player.levelData[level].frameCount * FRAME_TIME;
  } else {
    g_player.levelData[level].time = g_player.time;
  }

  g_player.levelData[level].score = g_player.score - g_player.previousScore;
  g_player.previousScore          = g_player.score;

  g_player.levelData[level].clearResult = scores_levelClear(
      g_player.levelData[level].time, g_player.levelData[level].score
  );

  if (level == LEVEL_COUNT - 1) {
    g_player.fullRun.time  = 0.0;
    g_player.fullRun.score = 0;
    for (int i = 0; i < LEVEL_COUNT; i++) {
      if (game_getDifficulty() == DIFFICULTY_ARCADE) {
        g_player.fullRun.time += g_player.levelData[level].frameCount * FRAME_TIME;
      } else {
        g_player.fullRun.time += g_player.levelData[level].time;
      }
      g_player.fullRun.score += g_player.levelData[level].score;
    }
    g_player.fullRun.clearResult = scores_fullRun(g_player.fullRun.time, g_player.fullRun.score);
  }
}

static void checkPickups(void) {
  // Check centre of tile, feels right.
  Vector2 pos = actor_getPos(g_player.actor);
  pos         = Vector2AddValue(pos, ACTOR_SIZE / 2.0f);
  if (maze_isCoin(pos)) {
    maze_pickupCoin(pos);
    coinPickup();
  } else if (maze_isSword(pos)) {
    maze_pickupSword(pos);
    swordPickup();
    creature_swordPickup();
  } else if (maze_isChest(pos)) {
    maze_pickupChest(pos, getChestScore());
    chestPickup();
  } else if (maze_isKey(pos)) {
    maze_pickupKey(pos);
    keyPickup();
  }

  if (maze_getCoinCount() == g_player.coinsCollected) {
    levelClear();
    game_levelClear();
  }
}

static void deadCommon(void) {
  g_player.lives     -= 1;
  g_player.deadTimer  = PLAYER_DEAD_TIMER;
  audio_resetChimePitch();
}

static void fallToDeath(void) {
  if (debug_isPlayerImmune()) return;

  deadCommon();
  g_player.state = PLAYER_FALLING;
  audio_playFalling(player_getPos());
}

static bool checkTraps(void) {
  // Wait till player is right on top of the trap
  Vector2 pos = actor_getPos(g_player.actor);
  switch (player_getDir()) {
    case DIR_UP: pos.y += ACTOR_SIZE - 1; break;
    case DIR_RIGHT: break;
    case DIR_DOWN: break;
    case DIR_LEFT: pos.x += ACTOR_SIZE - 1; break;
    default: assert(false);
  }
  if (maze_isTrapDoor(pos)) {
    maze_trapTriggered(pos);
    fallToDeath();
    return true;
  } else if (maze_isTrap(pos)) {
    maze_trapTriggered(pos);
    player_dead();
    return true;
  }
  return false;
}

static void checkScore() {
  int threshold = SCORE_EXTRA_LIFE[game_getDifficulty()];
  if (g_player.score >= g_player.lastScoreBonusLife + threshold) {
    g_player.lastScoreBonusLife += threshold;
    if (g_player.lives < PLAYER_MAX_LIVES) {
      g_player.lives += 1;
      LOG_INFO(game_log, "Player gained bonus life at score: %d", g_player.score);
    }
  }
}

// --- Player functions ---

bool player_init(void) {
  assert(g_player.actor == nullptr);
  g_player.actor = actor_create(
      PLAYER_START_POS,
      (Vector2) { ACTOR_SIZE, ACTOR_SIZE },
      PLAYER_START_DIR,
      PLAYER_MAX_SPEED[game_getDifficulty()],
      true
  );
  return g_player.actor != nullptr;
}

void player_shutdown(void) {
  assert(g_player.actor != nullptr);
  actor_destroy(&g_player.actor);
  assert(g_player.actor == nullptr);
}

void player_ready(void) {
  g_player.previousTime     = engine_getTime();
  g_player.time             = 0.0;
  int level                 = game_getLevel();
  g_player.levelData[level] = (player_levelData) {};
  g_accumulator             = 0.0;
}

void player_update(double frameTime, float slop) {
  assert(g_player.actor != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= 0.0f);

  if (game_getDifficulty() == DIFFICULTY_ARCADE) g_player.levelData[game_getLevel()].frameCount++;

#ifndef NDEBUG
  if (input_isKeyPressed(INPUT_F)) debug_toggleFPSOverlay();
  if (input_isKeyPressed(INPUT_M)) debug_toggleMazeOverlay();
  if (input_isKeyPressed(INPUT_P)) debug_togglePlayerOverlay();
  if (input_isKeyPressed(INPUT_C)) debug_toggleCreatureOverlay();
  if (input_isKeyPressed(INPUT_I)) debug_togglePlayerImmune();
  if (input_isKeyPressed(INPUT_N)) game_nextLevel();
#endif

  if (g_player.state == PLAYER_DEAD || g_player.state == PLAYER_FALLING) {
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

  coinSlowUpdate(frameTime);
  swordUpdate(frameTime);
  swordSlowUpdate(frameTime);

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

  if (checkTraps()) return;  // Dead!
  checkPickups();
  checkScore();
}

// Player dead
void player_restart(void) {
  assert(g_player.actor != nullptr);
  g_player.deadTimer       = 0.0f;
  g_player.state           = PLAYER_NORMAL;
  g_player.coinSlowTimer   = 0.0f;
  g_player.swordTimer      = 0.0f;
  g_player.swordSlowTimer  = 0.0f;
  g_player.scoreMultiplier = 1;
  actor_setPos(g_player.actor, PLAYER_START_POS);
  actor_setDir(g_player.actor, PLAYER_START_DIR);
  actor_setSpeed(g_player.actor, PLAYER_MAX_SPEED[game_getDifficulty()]);
  actor_startMoving(g_player.actor);
  audio_resetChimePitch();
}

// Next level
void player_reset() {
  player_restart();
  g_player.coinsCollected             = 0;
  g_player.levelData[game_getLevel()] = (player_levelData) {};
}

// New game
void player_totalReset() {
  player_reset();
  g_player.lives              = PLAYER_LIVES;
  g_player.score              = 0;
  g_player.previousScore      = 0;
  g_player.lastScoreBonusLife = 0;
}

void player_onPause(void) { g_player.time += engine_getTime() - g_player.previousTime; }

void player_onResume(void) { g_player.previousTime = engine_getTime(); }

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

game_Actor* player_getActor(void) {
  assert(g_player.actor != nullptr);
  return g_player.actor;
}

Vector2 player_getPos(void) {
  assert(g_player.actor != nullptr);
  return actor_getPos(g_player.actor);
}

game_Dir player_getDir(void) {
  assert(g_player.actor != nullptr);
  return actor_getDir(g_player.actor);
}

float player_getMaxSpeed(void) {
  // Player is faster in easy but we don't want creatures any faster
  return PLAYER_MAX_SPEED[DIFFICULTY_NORMAL];
}

player_levelData player_getLevelData(void) { return g_player.levelData[game_getLevel()]; }
player_levelData player_getFullRunData(void) { return g_player.fullRun; }

int player_getLives(void) {
  assert(g_player.lives >= 0 && g_player.lives <= PLAYER_MAX_LIVES);
  return g_player.lives;
}

int player_getScore(void) {
  assert(g_player.actor != nullptr);
  return g_player.score;
}

game_PlayerState player_getState(void) {
  assert(g_player.state >= 0 && g_player.state < PLAYER_STATE_COUNT);
  return g_player.state;
}

float player_getSwordTimer(void) {
  assert(g_player.swordTimer >= 0.0f);
  return g_player.swordTimer;
}

int player_getCoinsCollected(void) {
  assert(g_player.coinsCollected >= 0);
  return g_player.coinsCollected;
}

int player_getNextExtraLifeScore(void) {
  game_Difficulty difficulty = game_getDifficulty();
  return (g_player.score / SCORE_EXTRA_LIFE[difficulty] + 1) * SCORE_EXTRA_LIFE[difficulty];
}

bool player_isMoving(void) {
  assert(g_player.actor != nullptr);
  return actor_isMoving(g_player.actor);
}

bool player_hasSword(void) {
  assert(g_player.swordTimer >= 0.0f);
  return g_player.swordTimer > 0.0f;
}

void player_dead(void) {
  if (debug_isPlayerImmune()) return;
  player_onPause();

  deadCommon();
  g_player.state = PLAYER_DEAD;
  audio_playDeath(player_getPos());
}

void player_killedCreature(int creatureID) {
  assert(creatureID >= 0 && creatureID < CREATURE_COUNT);

  int score                 = creature_BASE_SCORE * g_player.scoreMultiplier;
  g_player.score           += score;
  g_player.scoreMultiplier *= 2;
  creature_setScore(creatureID, score);
}
