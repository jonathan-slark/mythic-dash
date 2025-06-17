/*
 * ghost.h: Internal to maze units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../game.h"
#include <raylib.h>

// --- Types ---

typedef enum ghost__GhostSpeed {
  SpeedSlow,
  SpeedNormal,
  SpeedFast
} ghost__GhostSpeed;

typedef struct ghost__Ghost {
  void (*update)(struct ghost__Ghost *, float, float);
  float timer;
  Vector2 mazeStart;
  float decisionCooldown;
  game__Actor *actor;

#ifndef NDEBUG
  unsigned id;
#endif // NDEBUG

} ghost__Ghost;

// --- Ghost state function prototypes ---

void ghost__pen(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__penToStart(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__wander(ghost__Ghost *ghost, float frameTime, float slop);
void ghost__chase(ghost__Ghost *ghost, float frameTime, float slop);

// --- Constants ---

static const float SPEEDS[] = {25.0f, 40.0f, 50.f, 100.0f};
static const float DECISION_COOLDOWN = 0.5f;

static const Vector2 GHOST_MAZE_START[] = {{11 * TILE_SIZE, 7 * TILE_SIZE},
                                           {17 * TILE_SIZE, 7 * TILE_SIZE}};
static const game__Dir GHOST_START_DIR = DIR_LEFT;
static const float GHOST_CHASETIMER = 10.0f;
static const struct {
  Vector2 startPos;
  Vector2 mazeStart;
  game__Dir startDir;
  float startSpeed;
  float startTimer;
  void (*update)(ghost__Ghost *, float, float);
} CREATURE_DATA[CREATURE_COUNT] = {
    [0] = {{13 * TILE_SIZE, 6 * TILE_SIZE},
           GHOST_MAZE_START[0],
           DIR_DOWN,
           SPEEDS[SpeedSlow],
           GHOST_CHASETIMER * 0.0f,
           ghost__pen},
    [1] = {{15 * TILE_SIZE, 6 * TILE_SIZE},
           GHOST_MAZE_START[1],
           DIR_LEFT,
           SPEEDS[SpeedSlow],
           GHOST_CHASETIMER * 1.0f,
           ghost__pen},
    [2] = {{13 * TILE_SIZE, 9 * TILE_SIZE},
           GHOST_MAZE_START[0],
           DIR_RIGHT,
           SPEEDS[SpeedSlow],
           GHOST_CHASETIMER * 2.0f,
           ghost__pen},
    [3] = {{15 * TILE_SIZE, 9 * TILE_SIZE},
           GHOST_MAZE_START[1],
           DIR_UP,
           SPEEDS[SpeedSlow],
           GHOST_CHASETIMER * 3.0f,
           ghost__pen},
};

// --- Global state ---

extern ghost__Ghost g_ghosts[];
