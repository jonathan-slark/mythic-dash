#include "creature.h"
#include <assert.h>
#include <limits.h>
#include <raylib.h>
#include "../actor/actor.h"
#include "../audio/audio.h"
#include "../internal.h"
#include "../maze/maze.h"
#include "../player/player.h"
#include "internal.h"
#include "log/log.h"

// --- Constants ---

constexpr int      STATE_COUNT                                     = 7;
static const float STATE_TIMERS_MIN[DIFFICULTY_COUNT][STATE_COUNT] = {
  { 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f, 15.0f },
  { 10.0f, 15.0f, 10.0f, 15.0f, 10.0f, 15.0f, 10.0f },
  {  7.0f, 20.0f,  7.0f, 20.0f,  5.0f, 20.0f,  5.0f }
};
static const float STATE_TIMERS_MAX[DIFFICULTY_COUNT][STATE_COUNT] = {
  { 12.0f, 15.0f, 12.0f, 15.0f, 12.0f, 15.0f, 12.0f },
  {  7.0f, 20.0f,  7.0f, 20.0f,  5.0f, 20.0f,  5.0f },
  {  3.0f, 20.0f,  3.0f, 20.0f,  1.0f, 20.0f,  1.0f }
};
static const char* STATE_PEN_STR        = "PEN";
static const char* STATE_PETTOSTART_STR = "PEN2STA";
static const char* STATE_STARTTOPEN_STR = "STA2PEN";
static const char* STATE_FRIGHTENED_STR = "FRIGHT";
static const char* STATE_DEAD_STR       = "DEAD";
static const char* STATE_CHASE_STR      = "CHASE";
static const char* STATE_SCATTER_STR    = "SCATTER";
static const float SCORE_TIMER          = 2.0f;
static const float TELEPORT_TIMER       = 0.5f;

// --- Global state ---

creature_State g_state = { .update = nullptr, .lastUpdate = nullptr, .stateNum = 0, .stateTimer = 0.0f };

// --- Helper functions ---

static inline float isDead(creature_Creature* creature) { return creature->update == creature_dead; }

static inline void updateTimer(float frameTime) {
  assert(frameTime >= 0.0f);
  g_state.stateTimer = fmaxf(g_state.stateTimer - frameTime, 0.0f);
}
static inline bool isPermanentChaseState(void) { return g_state.stateNum == STATE_COUNT; }

static inline bool shouldTransitionState(void) { return g_state.stateTimer == 0.0f && g_state.stateNum <= STATE_COUNT; }

static inline bool isInActiveState(creature_Creature* creature) {
  assert(creature != nullptr);
  return creature->update == creature_chase || creature->update == creature_scatter ||
         creature->update == creature_frightened;
}

static const char* getStateString(void (*update)(struct creature_Creature*, float, float)) {
  assert(update != nullptr);

  if (update == creature_pen) return STATE_PEN_STR;
  if (update == creature_penToStart) return STATE_PETTOSTART_STR;
  if (update == creature_startToPen) return STATE_STARTTOPEN_STR;
  if (update == creature_frightened) return STATE_FRIGHTENED_STR;
  if (update == creature_dead) return STATE_DEAD_STR;
  if (update == creature_chase) return STATE_CHASE_STR;
  if (update == creature_scatter) return STATE_SCATTER_STR;
  return "";
}

static void transitionToState(void (*newState)(creature_Creature*, float, float)) {
  assert(newState != nullptr);

  if (g_state.update != creature_frightened) g_state.lastUpdate = g_state.update;
  g_state.update = newState;
  for (int i = 0; i < CREATURE_COUNT; i++) {
    if (isInActiveState(&g_state.creatures[i])) {
      g_state.creatures[i].update         = newState;
      g_state.creatures[i].isChangedState = true;
    }
  }
}

static void transitionToPermanentChase(void) {
  transitionToState(creature_chase);
  g_state.lastUpdate = creature_chase;
  g_state.stateNum++;
  assert(g_state.stateNum == STATE_COUNT + 1);
}

static float getStateTimer(int stateNum) {
  game_Difficulty difficulty = game_getDifficulty();
  assert(difficulty >= 0 && difficulty < DIFFICULTY_COUNT);

  float minTimer = STATE_TIMERS_MIN[difficulty][stateNum];
  float maxTimer = STATE_TIMERS_MAX[difficulty][stateNum];

  // Get current level-based interpolation factor
  // Level 1 → t = 0.0, Level LEVEL_COUNT → t = 1.0
  int level = game_getLevel();
  assert(level >= 0 && level < LEVEL_COUNT);
  float t = (float) level / (LEVEL_COUNT - 1);  // normalised [0,1]

  return minTimer + t * (maxTimer - minTimer);
}

static void toggleCreatureState() {
  void (*newState)(
      creature_Creature*, float, float
  ) = (g_state.update == nullptr || g_state.update == creature_chase) ? creature_scatter : creature_chase;
  transitionToState(newState);
  g_state.stateTimer = getStateTimer(g_state.stateNum++);
  assert(g_state.stateNum <= STATE_COUNT);
  LOG_TRACE(game_log, "Changing to state: %s", getStateString(newState));
}

// Update the global state and change creature states
static void updateState(float frameTime) {
  assert(frameTime >= 0.0f);

  updateTimer(frameTime);

  if (shouldTransitionState()) {
    if (isPermanentChaseState()) {
      transitionToPermanentChase();
    } else {
      toggleCreatureState();
    }
  }
}

static void creatureResetTargets(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    g_state.creatures[i].targetTile = DEFAULT_TARGET_TILE;
  }
}

static void creatureResetTeleportTimer(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    g_state.creatures[i].teleportTimer = 0.0f;
  }
}

static void creatureStopWhispers(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    if (g_state.creatures[i].whisperId != -1) audio_stopWhispers(&g_state.creatures[i].whisperId);
  }
}

static void creatureWail(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    if (isInActiveState(&g_state.creatures[i])) audio_playWail(actor_getPos(g_state.creatures[i].actor));
  }
}

static void creatureDefaults(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    g_state.creatures[i].update           = CREATURE_DATA[i].update;
    g_state.creatures[i].startTimer       = CREATURE_DATA[i].startTimer;
    g_state.creatures[i].mazeStart        = CREATURE_DATA[i].mazeStart;
    g_state.creatures[i].cornerTile       = CREATURE_DATA[i].cornerTile;
    g_state.creatures[i].decisionCooldown = 0.0f;
    g_state.creatures[i].score            = 0;
    g_state.creatures[i].scoreTimer       = 0.0f;
    g_state.creatures[i].teleportTimer    = 0.0f;
    g_state.creatures[i].whisperId        = -1;
  }
  actor_setSpeed(g_state.creatures[1].actor, creature_getSpeed(&g_state.creatures[1]));

  creatureResetTargets();
}

static void creatureSetSpeeds(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    if (isInActiveState(&g_state.creatures[i]))
      actor_setSpeed(g_state.creatures[i].actor, creature_getSpeed(&g_state.creatures[i]));
  }
}

static void creatureCheckScoreTimer(creature_Creature* creature, float frameTime) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);

  if (creature->scoreTimer == 0.0f) return;

  creature->scoreTimer = fmaxf(creature->scoreTimer - frameTime, 0.0f);
  if (creature->scoreTimer == 0.0f) creature->score = 0;
}

static void creatureCheckTeleport(creature_Creature* creature) {
  assert(creature != nullptr);

  if (actor_hasTeleported(creature->actor)) {
    creature->teleportTimer = TELEPORT_TIMER;
    actor_setSpeed(creature->actor, SPEED_SLOW);
  }
}

static void creatureUpdateTeleportSlow(creature_Creature* creature, float frameTime) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);

  if (creature->teleportTimer == 0.0f) return;

  creature->teleportTimer = fmaxf(creature->teleportTimer - frameTime, 0.0f);
  if (creature->teleportTimer == 0.0f) actor_setSpeed(creature->actor, creature_getSpeed(creature));
}

static void creatureSetNearestStartTile(creature_Creature* creature) {
  assert(creature != nullptr);

  game_Tile curTile    = maze_getTile(actor_getPos(creature->actor));
  size_t    startCount = COUNT(creature_MAZE_START);
  size_t    bestTiles[startCount];
  size_t    bestTileCount = 0;
  int       minDist       = INT_MAX;

  for (size_t i = 0; i < startCount; i++) {
    game_Tile destTile = maze_getTile(creature_MAZE_START[i]);
    int       dist     = maze_manhattanDistance(curTile, destTile);
    if (dist < minDist) {
      bestTileCount = 0;
    }
    if (dist <= minDist) {
      bestTiles[bestTileCount++] = i;
      minDist                    = dist;
    }
  }

  assert(bestTileCount > 0 && bestTileCount <= startCount);
  assert(minDist >= 0 && minDist < INT_MAX);

  if (bestTileCount == 1) {
    creature->mazeStart = creature_MAZE_START[bestTiles[0]];
    LOG_TRACE(game_log, "Creature %d best start tile %d (best choice)", creature->id, bestTiles[0]);
  } else {
    size_t bestTile     = bestTiles[GetRandomValue(0, bestTileCount - 1)];
    creature->mazeStart = creature_MAZE_START[bestTile];
    LOG_TRACE(game_log, "Creature %d best start tile %d (%d choices)", creature->id, bestTile, bestTileCount);
  }
}

static void creatureDied(creature_Creature* creature) {
  assert(creature != nullptr);

  creature->update = creature_dead;
  actor_setSpeed(creature->actor, creature_getSpeed(creature));
  creature->teleportTimer = 0.0f;
  creatureSetNearestStartTile(creature);
  player_killedCreature(creature->id);
  creature->whisperId = audio_playWhispers(actor_getPos(creature->actor));
}

// --- Creature functions ---

bool creature_init(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_state.creatures[i].actor == nullptr);
    g_state.creatures[i].actor = actor_create(
        CREATURE_DATA[i].startPos,
        (Vector2) { ACTOR_SIZE, ACTOR_SIZE },
        CREATURE_DATA[i].startDir,
        CREATURE_DATA[i].startSpeed,
        false
    );
    if (g_state.creatures[i].actor == nullptr) return false;
    g_state.creatures[i].id = i;
  }

  // creatureDefaults();
  return true;
}

void creature_reset(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    actor_setPos(g_state.creatures[i].actor, CREATURE_DATA[i].startPos);
    actor_setDir(g_state.creatures[i].actor, CREATURE_DATA[i].startDir);
    actor_setSpeed(g_state.creatures[i].actor, CREATURE_DATA[i].startSpeed);
  }

  creatureDefaults();
  creatureStopWhispers();
  g_state.update     = nullptr;
  g_state.stateNum   = 0;
  g_state.stateTimer = 0.0f;
}

void creature_shutdown(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_state.creatures[i].actor != nullptr);
    actor_destroy(&g_state.creatures[i].actor);
    assert(g_state.creatures[i].actor == nullptr);
  }
}

void creature_update(float frameTime, float slop) {
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  bool             playerDead  = false;
  game_PlayerState playerState = player_getState();

  for (int i = 0; i < CREATURE_COUNT; i++) {
    g_state.creatures[i].update(&g_state.creatures[i], frameTime, slop);

    if (isInActiveState(&g_state.creatures[i])) {
      if (actor_isColliding(player_getActor(), g_state.creatures[i].actor)) {
        if (playerState == PLAYER_SWORD) {
          creatureDied(&g_state.creatures[i]);
        } else {
          playerDead = true;
        }
      }
    }

    creatureCheckScoreTimer(&g_state.creatures[i], frameTime);

    if (g_state.creatures[i].update != creature_frightened && g_state.creatures[i].update != creature_dead) {
      creatureCheckTeleport(&g_state.creatures[i]);
      creatureUpdateTeleportSlow(&g_state.creatures[i], frameTime);
    }

    if (g_state.creatures[i].whisperId != -1) {
      audio_updateWhispers(g_state.creatures[i].whisperId);
    }
  }

  if (!player_hasSword()) updateState(frameTime);

  if (playerDead && playerState != PLAYER_DEAD && playerState != PLAYER_FALLING) player_dead();
}

Vector2 creature_getPos(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.creatures[id].actor != nullptr);
  return actor_getPos(g_state.creatures[id].actor);
}

game_Dir creature_getDir(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.creatures[id].actor != nullptr);
  return actor_getDir(g_state.creatures[id].actor);
}

game_Actor* creature_getActor(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.creatures[id].actor != nullptr);
  return g_state.creatures[id].actor;
}

float creature_getDecisionCooldown(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.creatures[id].actor != nullptr);
  return g_state.creatures[id].decisionCooldown;
}

game_Tile creature_getTarget(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  return g_state.creatures[id].targetTile;
}

float creature_getGlobalTimer(void) { return player_hasSword() ? player_getSwordTimer() : g_state.stateTimer; }

int creature_getGlobaStateNum(void) { return g_state.stateNum; }

const char* creature_getGlobalStateString(void) {
  const char* string = getStateString(g_state.update);
  assert(string != nullptr);
  return string;
}

const char* creature_getStateString(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.creatures[id].update != nullptr);

  return getStateString(g_state.creatures[id].update);
}

void creature_swordPickup(void) {
  transitionToState(creature_frightened);
  creatureSetSpeeds();
  creatureResetTargets();
  creatureResetTeleportTimer();
  creatureWail();
}

void creature_swordDrop(void) {
  assert(g_state.lastUpdate != nullptr);
  transitionToState(g_state.lastUpdate);
  creatureSetSpeeds();
}

bool creature_isFrightened(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.creatures[id].update != nullptr);
  return g_state.creatures[id].update == creature_frightened;
}

bool creature_isDead(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.creatures[id].update != nullptr);
  return g_state.creatures[id].update == creature_dead || g_state.creatures[id].update == creature_startToPen;
}

void creature_setScore(int id, int score) {
  assert(id >= 0 && id < CREATURE_COUNT);
  g_state.creatures[id].score      = score;
  g_state.creatures[id].scoreTimer = SCORE_TIMER;
}

int creature_getScore(int id) { return g_state.creatures[id].score; }

float creature_getSpeed(creature_Creature* creature) {
  assert(creature != nullptr);

  game_Difficulty difficulty = game_getDifficulty();
  assert(difficulty >= 0 && difficulty < DIFFICULTY_COUNT);

  float minMult, maxMult;

  if (isDead(creature)) {
    minMult = RETREAT_SPEED_MIN_MULT[difficulty];
    maxMult = RETREAT_SPEED_MAX_MULT[difficulty];
  } else if (player_hasSword()) {
    minMult = FRIGHT_SPEED_MIN_MULT[difficulty];
    maxMult = FRIGHT_SPEED_MAX_MULT[difficulty];
  } else {
    minMult = NORMAL_SPEED_MIN_MULT[difficulty];
    maxMult = NORMAL_SPEED_MAX_MULT[difficulty];
  }

  // Get current level-based interpolation factor
  // Level 1 → t = 0.0, Level LEVEL_COUNT → t = 1.0
  int level = game_getLevel();
  assert(level >= 0 && level < LEVEL_COUNT);
  float t = (float) level / (LEVEL_COUNT - 1);  // normalised [0,1]

  float speedMultiplier = minMult + t * (maxMult - minMult);
  return player_getMaxSpeed() * speedMultiplier;
}
