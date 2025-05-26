// clang-format Language: C
#pragma once

#include <raylib.h>
#include <raymath.h>
#include "../engine/engine.h"
#include "../log/log.h"

// --- Helper macros ---

#define GAME_TRY(func) \
  if (!(func)) {       \
    return false;      \
  }

#define POS_ADJUST(pos) Vector2Add((pos), MAZE_ORIGIN)
#define OVERLAY_COLOUR_PLAYER (Color){100, 200, 255, 128}
#define OVERLAY_COLOUR_TILE_WALL (Color){255, 100, 100, 128}
#define OVERLAY_COLOUR_TILE_FLOOR (Color){255, 100, 100, 32}
#define OVERLAY_COLOUR_COLLISION (Color){255, 255, 100, 128}

// --- Types ---

typedef enum Dir { DIR_NONE, DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_COUNT } Dir;

typedef struct AABB {
  Vector2 min;
  Vector2 max;
} AABB;

typedef struct Actor Actor;

// --- Constants ---

constexpr int        ACTOR_SIZE = 16;
extern const Vector2 MAZE_ORIGIN;
extern const float   TILE_SIZE;

// --- Global state ---

extern log_Log* game__log;
extern bool     game__isOverlayEnabled;

// --- Helper functions ---

static inline bool aabb_isColliding(AABB a, AABB b) {
  return a.min.x < b.max.x && a.max.x > b.min.x && a.min.y < b.max.y && a.max.y > b.min.y;
}

static inline float aabb_getOverlapX(AABB a, AABB b) { return fmin(a.max.x, b.max.x) - fmax(a.min.x, b.min.x); }

static inline float aabb_getOverlapY(AABB a, AABB b) { return fmin(a.max.y, b.max.y) - fmax(a.min.y, b.min.y); }

#ifndef NDEBUG
static inline void aabb_drawOverlay(AABB aabb, Color colour) {
  Vector2 min = POS_ADJUST(aabb.min);
  Vector2 max = POS_ADJUST(aabb.max);
  engine_drawRectangleOutline((Rectangle) {min.x, min.y, max.x - min.x, max.y - min.y}, colour);
}
#endif

// --- Actor functions (actor.c) ---

Actor*  actor_create(Vector2 pos, Vector2 size, Dir dir, float speed);
void    actor_destroy(Actor** actor);
Dir     actor_getDir(const Actor* actor);
Vector2 actor_getPos(const Actor* actor);
Vector2 actor_getSize(const Actor* actor);
AABB    actor_getAABB(const Actor* actor);
bool    actor_canMove(Actor* actor, Dir dir);
void    actor_overlay(const Actor* actor, Color colour);
void    actor_wallsOverlay(Actor* actor);
void    actor_move(Actor* actor, Dir dir, float frameTime);

// --- Player functions (player.c) ---

bool    player_init(void);
void    player_shutdown(void);
void    player_update(float frameTime);
Vector2 player_getPos(void);
void    player_overlay(void);

// --- Maze functions (maze.c) ---

void maze_init(void);
AABB maze_getAABB(Vector2 pos);
bool maze_isWall(Vector2 pos);
