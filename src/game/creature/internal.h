/*
 * creature.h: Internal to creature units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../internal.h"
#include "../player/player.h"
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

void creature__pen(creature_Creature *creature, float frameTime, float slop);
void creature__penToStart(creature_Creature *creature, float frameTime,
                          float slop);
void creature__startToPen(creature_Creature *creature, float frameTime,
                          float slop);
void creature__frightened(creature_Creature *creature, float frameTime,
                          float slop);
void creature__dead(creature_Creature *creature, float frameTime, float slop);
void creature__chase(creature_Creature *creature, float frameTime, float slop);
void creature__scatter(creature_Creature *creature, float frameTime,
                       float slop);

// --- Constants ---

constexpr float SPEED_SLOW = 40.0f; // 50% of player max speed
static const float NORMAL_SPEED_MIN_MULT = 0.75f;
static const float NORMAL_SPEED_MAX_MULT = 0.95f;
static const float NORMAL_LEVEL_MULT = 0.011f;
static const float FRIGHT_SPEED_MIN_MULT = 0.50f;
static const float FRIGHT_SPEED_MAX_MULT = 0.60f;
static const float FRIGHT_LEVEL_MULT = 0.0055f;
static const float DECISION_COOLDOWN = 0.2f;

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
           creature__pen},
    [1] = {{17 * TILE_SIZE, 7 * TILE_SIZE},
           creature_MAZE_START[1],
           {27, 1},
           DIR_UP,
           SPEED_SLOW,
           0.0f,
           creature__scatter},
    [2] = {{13 * TILE_SIZE, 8 * TILE_SIZE},
           creature_MAZE_START[0],
           {1, 13},
           DIR_UP,
           SPEED_SLOW,
           creature_CHASETIMER * 0.0f,
           creature__pen},
    [3] = {{14 * TILE_SIZE, 7 * TILE_SIZE},
           creature_MAZE_START[0],
           {27, 13},
           DIR_DOWN,
           SPEED_SLOW,
           creature_CHASETIMER * 2.0f,
           creature__pen},
};

// --- Global state ---

extern creature_State g_state;

// --- Helper functions ---

// Level 01: normal creature speed = 75% of player, fright creature speed = 50%
// Level 20: normal creature speed = 95% of player, fright creature speed = 60%
static inline float creature__getSpeed(void) {
  if (player_hasSword()) {
    return fminf(FRIGHT_SPEED_MIN_MULT +
                     (game_getLevel() - 1) * FRIGHT_LEVEL_MULT,
                 FRIGHT_SPEED_MAX_MULT) *
           player_getMaxSpeed();
  } else {
    return fminf(NORMAL_SPEED_MIN_MULT +
                     (game_getLevel() - 1) * NORMAL_LEVEL_MULT,
                 NORMAL_SPEED_MAX_MULT) *
           player_getMaxSpeed();
  }
}
