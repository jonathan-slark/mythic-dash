/*
 * ghost.h: Internal to ghost units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../game.h"
#include <raylib.h>

// --- Types ---

typedef struct ghost__Ghost {
  void (*update)(struct ghost__Ghost *, float, float);
  float timer;
  Vector2 mazeStart;
  game__Tile cornerTile;
  game__Tile targetTile;
  float decisionCooldown;
  game__Actor *actor;
  unsigned id;
  bool isChangedState;
} ghost__Ghost;

typedef struct ghost__State {
  ghost__Ghost ghosts[CREATURE_COUNT];
  void (*update)(ghost__Ghost *, float, float);
  size_t stateNum;
  float stateTimer;
} ghost__State;

// --- Ghost state function prototypes ---

void ghost__pen(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__penToStart(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__frightened(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__dead(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__chase(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__scatter(ghost__Ghost *ghost, float frameTime, float slop);

// --- Constants ---

constexpr float SPEED_SLOW = 25.0f;
static const float SPEED_MIN_MULT = 0.75f;
static const float SPEED_MAX_MULT = 0.95f;
static const float DECISION_COOLDOWN = 0.5f;

static const int FIRST_GHOST_OUT = 1;
static const Vector2 GHOST_MAZE_START[] = {{11 * TILE_SIZE, 7 * TILE_SIZE},
                                           {17 * TILE_SIZE, 7 * TILE_SIZE}};
static const game__Dir GHOST_START_DIR = DIR_LEFT;
static const float GHOST_CHASETIMER = 10.0f;
static const game__Tile DEFAULT_TARGET_TILE = {-1, -1};
static const struct {
  Vector2 startPos;
  Vector2 mazeStart;
  game__Tile cornerTile;
  game__Dir startDir;
  float startSpeed;
  float startTimer;
  void (*update)(ghost__Ghost *, float, float);
} CREATURE_DATA[CREATURE_COUNT] = {
    [0] = {{13 * TILE_SIZE, 6 * TILE_SIZE},
           GHOST_MAZE_START[0],
           {1, 1},
           DIR_RIGHT,
           SPEED_SLOW,
           GHOST_CHASETIMER * 2.0f,
           ghost__pen},
    [1] = {{15 * TILE_SIZE, 7 * TILE_SIZE},
           GHOST_MAZE_START[1],
           {27, 1},
           DIR_LEFT,
           SPEED_SLOW,
           GHOST_CHASETIMER * 0.0f,
           ghost__pen},
    [2] = {{13 * TILE_SIZE, 8 * TILE_SIZE},
           GHOST_MAZE_START[0],
           {1, 13},
           DIR_RIGHT,
           SPEED_SLOW,
           GHOST_CHASETIMER * 1.0f,
           ghost__pen},
    [3] = {{15 * TILE_SIZE, 9 * TILE_SIZE},
           GHOST_MAZE_START[1],
           {27, 13},
           DIR_LEFT,
           SPEED_SLOW,
           GHOST_CHASETIMER * 3.0f,
           ghost__pen},
};

// --- Global state ---

extern ghost__State g_state;
