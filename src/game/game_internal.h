// clang-format Language: C
#pragma once

#include <raylib.h>  // Vector2
#include "../log/log.h"

// --- Helper macros ---

#define GAME_TRY(func) \
  if (!(func)) {       \
    return false;      \
  }

#define POS_ADJUST(pos) Vector2Add(pos, MAZE_ORIGIN)
#define OVERLAY_COLOUR (Color){100, 200, 255, 128}

// --- Types ---

typedef enum Dir { Up, Right, Down, Left, None } Dir;

typedef struct Actor {
  Vector2 pos;
  Dir     dir;
} Actor;

typedef struct AABB {
  Vector2 min;
  Vector2 max;
} AABB;

// --- Constants ---

constexpr int        ACTOR_SIZE = 16;
extern const Vector2 MAZE_ORIGIN;
extern const Vector2 VELS[];

// --- Global state ---

extern log_Log* game__log;
extern bool     game__isOverlayEnabled;

// --- Actor functions (actor.c) ---

AABB game__getActorAABB(Actor actor);

// --- Player functions (player.c) ---

void    game__playerInit(void);
void    game__playerUpdate(float frameTime);
Vector2 game__playerGetPos(void);
void    game__playerOverlay(void);

// --- Maze functions (maze.c) ---

bool game__mazeInit(void);
void game__mazeUninit(void);
bool game__isHittingWall(AABB aabb);
void game__mazeOverlay(void);
