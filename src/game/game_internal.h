// clang-format Language: C
#pragma once

#include <raylib.h>  // Vector2
#include "../log/log.h"

// --- Helper macros ---

#define GAME_TRY(func) \
  if (!(func)) {       \
    return false;      \
  }

// --- Types ---

typedef enum { Up, Right, Down, Left, None } Dir;

typedef struct {
  Vector2 pos;
  Dir dir;
} Actor;

// --- Constants ---

extern const int ACTOR_SIZE;
extern const Vector2 MAZE_ORIGIN;
extern const Vector2 VELS[];

// --- Global state ---

extern log_Log* game__log;

// --- Actor functions (actor.c) ---

bool game__actorCanMove(Actor actor, float distance);

// --- Player functions (player.c) ---

void game__playerInit(void);
void game__playerUpdate(float frameTime);
Vector2 game__playerGetPos(void);

// --- Maze functions (maze.c) ---

bool game__isMazeWall(float x, float y);
