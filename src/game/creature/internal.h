/*
 * creature.h: Internal to creature units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../internal.h"
#include "../player/player.h"
#include <assert.h>
#include <raylib.h>

// --- Types ---

typedef struct creature_Creature {
  void (*update)(struct creature_Creature *, float, float);
  float startTimer;
  Vector2 mazeStart;
  game_Tile cornerTile;
  game_Tile targetTile;
  float decisionCooldown;
  game_Actor *actor;
  unsigned id;
  bool isChangedState;
  int score;
  float scoreTimer;
  float teleportTimer;
  int whisperId;
} creature_Creature;

typedef struct creature_State {
  creature_Creature creatures[CREATURE_COUNT];
  void (*update)(creature_Creature *, float, float);
  void (*lastUpdate)(creature_Creature *, float, float);
  size_t stateNum;
  float stateTimer;
} creature_State;

// --- Creature state function prototypes ---

void creature_pen(creature_Creature *creature, float frameTime, float slop);
void creature_penToStart(creature_Creature *creature, float frameTime,
                         float slop);
void creature_startToPen(creature_Creature *creature, float frameTime,
                         float slop);
void creature_frightened(creature_Creature *creature, float frameTime,
                         float slop);
void creature_dead(creature_Creature *creature, float frameTime, float slop);
void creature_chase(creature_Creature *creature, float frameTime, float slop);
void creature_scatter(creature_Creature *creature, float frameTime, float slop);

// --- Constants ---

constexpr float SPEED_SLOW = 40.0f; // 50% of player max speed
static const float RETREAT_SPEED_MIN_MULT[DIFFICULTY_COUNT] = {0.50f, 0.67f,
                                                               1.0f};
static const float RETREAT_SPEED_MAX_MULT[DIFFICULTY_COUNT] = {0.67f, 0.8f,
                                                               1.2f};
static const float NORMAL_SPEED_MIN_MULT[DIFFICULTY_COUNT] = {0.63f, 0.69f,
                                                              0.75f};
static const float NORMAL_SPEED_MAX_MULT[DIFFICULTY_COUNT] = {0.69f, 0.75f,
                                                              0.95f};
static const float FRIGHT_SPEED_MIN_MULT[DIFFICULTY_COUNT] = {0.40f, 0.45f,
                                                              0.50f};
static const float FRIGHT_SPEED_MAX_MULT[DIFFICULTY_COUNT] = {0.450f, 0.50f,
                                                              0.60f};
static const float NORMAL_LEVEL_MULT = 0.011f;
static const float FRIGHT_LEVEL_MULT = 0.0055f;
static const float DECISION_COOLDOWN = 0.1f;

static const Vector2 MAZE_CENTRE = {14 * TILE_SIZE, 7 * TILE_SIZE};
static const Vector2 creature_MAZE_START[] = {{11 * TILE_SIZE, 7 * TILE_SIZE},
                                              {17 * TILE_SIZE, 7 * TILE_SIZE}};
static const game_Dir creature_START_DIR = DIR_LEFT;
static const float creature_CHASETIMER = 5.0f;
static const game_Tile DEFAULT_TARGET_TILE = {-1, -1};
static const struct {
  Vector2 startPos;
  Vector2 mazeStart;
  game_Tile cornerTile;
  game_Dir startDir;
  float startSpeed;
  float startTimer;
  void (*update)(creature_Creature *, float, float);
} CREATURE_DATA[CREATURE_COUNT] = {
    [0] = {{15 * TILE_SIZE, 8 * TILE_SIZE},
           creature_MAZE_START[1],
           {1, 1},
           DIR_UP,
           SPEED_SLOW,
           creature_CHASETIMER * 1.0f,
           creature_pen},
    [1] = {{17 * TILE_SIZE, 7 * TILE_SIZE},
           creature_MAZE_START[1],
           {27, 1},
           DIR_UP,
           SPEED_SLOW,
           0.0f,
           creature_scatter},
    [2] = {{13 * TILE_SIZE, 8 * TILE_SIZE},
           creature_MAZE_START[0],
           {1, 13},
           DIR_UP,
           SPEED_SLOW,
           creature_CHASETIMER * 0.0f,
           creature_pen},
    [3] = {{14 * TILE_SIZE, 7 * TILE_SIZE},
           creature_MAZE_START[0],
           {27, 13},
           DIR_DOWN,
           SPEED_SLOW,
           creature_CHASETIMER * 2.0f,
           creature_pen},
};

// --- Global state ---

extern creature_State g_state;

// --- Helper functions ---

static inline float creature_getSpeed(creature_Creature *creature) {
  assert(creature != nullptr);
  game_Difficulty difficulty = game_getDifficulty();

  if (creature->update == creature_dead) {
    return fminf(RETREAT_SPEED_MIN_MULT[difficulty] +
                     (game_getLevel() - 1) * FRIGHT_LEVEL_MULT,
                 RETREAT_SPEED_MAX_MULT[difficulty]) *
           player_getMaxSpeed();
  } else if (player_hasSword()) {
    return fminf(FRIGHT_SPEED_MIN_MULT[difficulty] +
                     (game_getLevel() - 1) * FRIGHT_LEVEL_MULT,
                 FRIGHT_SPEED_MAX_MULT[difficulty]) *
           player_getMaxSpeed();
  } else {
    return fminf(NORMAL_SPEED_MIN_MULT[difficulty] +
                     (game_getLevel() - 1) * NORMAL_LEVEL_MULT,
                 NORMAL_SPEED_MAX_MULT[difficulty]) *
           player_getMaxSpeed();
  }
}
