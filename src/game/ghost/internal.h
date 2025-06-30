/*
 * ghost.h: Internal to ghost units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../internal.h"
#include "../player/player.h"
#include <raylib.h>

// --- Types ---

typedef struct ghost__Ghost {
  void (*update)(struct ghost__Ghost *, float, float);
  float startTimer;
  Vector2 mazeStart;
  game_Tile cornerTile;
  game_Tile targetTile;
  float decisionCooldown;
  game__Actor *actor;
  unsigned id;
  bool isChangedState;
  int score;
  float scoreTimer;
  float teleportTimer;
} ghost__Ghost;

typedef struct ghost__State {
  ghost__Ghost ghosts[CREATURE_COUNT];
  void (*update)(ghost__Ghost *, float, float);
  void (*lastUpdate)(ghost__Ghost *, float, float);
  size_t stateNum;
  float stateTimer;
} ghost__State;

// --- Ghost state function prototypes ---

void ghost__pen(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__penToStart(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__startToPen(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__frightened(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__dead(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__chase(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__scatter(ghost__Ghost *ghost, float frameTime, float slop);

// --- Constants ---

constexpr float SPEED_SLOW = 40.0f; // 50% of player max speed
static const float NORMAL_SPEED_MIN_MULT = 0.75f;
static const float NORMAL_SPEED_MAX_MULT = 0.95f;
static const float NORMAL_LEVEL_MULT = 0.011f;
static const float FRIGHT_SPEED_MIN_MULT = 0.50f;
static const float FRIGHT_SPEED_MAX_MULT = 0.60f;
static const float FRIGHT_LEVEL_MULT = 0.0055f;
static const float DECISION_COOLDOWN = 0.5f;

static const Vector2 MAZE_CENTRE = {14 * TILE_SIZE, 7 * TILE_SIZE};
static const Vector2 GHOST_MAZE_START[] = {{11 * TILE_SIZE, 7 * TILE_SIZE},
                                           {17 * TILE_SIZE, 7 * TILE_SIZE}};
static const game_Dir GHOST_START_DIR = DIR_LEFT;
static const float GHOST_CHASETIMER = 5.0f;
static const game_Tile DEFAULT_TARGET_TILE = {-1, -1};
static const struct {
  Vector2 startPos;
  Vector2 mazeStart;
  game_Tile cornerTile;
  game_Dir startDir;
  float startSpeed;
  float startTimer;
  void (*update)(ghost__Ghost *, float, float);
} CREATURE_DATA[CREATURE_COUNT] = {
    [0] = {{15 * TILE_SIZE, 8 * TILE_SIZE},
           GHOST_MAZE_START[1],
           {1, 1},
           DIR_UP,
           SPEED_SLOW,
           GHOST_CHASETIMER * 1.0f,
           ghost__pen},
    [1] = {{17 * TILE_SIZE, 7 * TILE_SIZE},
           GHOST_MAZE_START[1],
           {27, 1},
           DIR_UP,
           SPEED_SLOW,
           0.0f,
           ghost__scatter},
    [2] = {{13 * TILE_SIZE, 8 * TILE_SIZE},
           GHOST_MAZE_START[0],
           {1, 13},
           DIR_UP,
           SPEED_SLOW,
           GHOST_CHASETIMER * 0.0f,
           ghost__pen},
    [3] = {{14 * TILE_SIZE, 7 * TILE_SIZE},
           GHOST_MAZE_START[0],
           {27, 13},
           DIR_DOWN,
           SPEED_SLOW,
           GHOST_CHASETIMER * 2.0f,
           ghost__pen},
};

// --- Global state ---

extern ghost__State g_state;

// --- Helper functions ---

// Level 01: normal ghost speed = 75% of player, fright ghost speed = 50%
// Level 20: normal ghost speed = 95% of player, fright ghost speed = 60%
static inline float ghost__getSpeed(void) {
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
