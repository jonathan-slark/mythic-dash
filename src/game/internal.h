// clang-format Language: C
#pragma once

#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include <raymath.h>

// --- Helper macros ---

#define GAME_TRY(func) \
  if (!(func)) {       \
    return false;      \
  }

#define COUNT(array) (sizeof(array) / sizeof(array[0]))
#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)
#define CLAMP(x, min, max) MIN(MAX((x), (min)), (max))
#define POS_ADJUST(pos) Vector2Add((pos), MAZE_ORIGIN)

// --- Types ---

typedef enum game_Dir { DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_COUNT, DIR_NONE } game_Dir;

typedef enum game_PlayerState {
  PLAYER_NORMAL,
  PLAYER_SWORD,
  PLAYER_DEAD,
  PLAYER_FALLING,
  PLAYER_STATE_COUNT
} game_PlayerState;

typedef struct game_AABB {
  Vector2 min;
  Vector2 max;
} game_AABB;

typedef struct game_Tile {
  int col;
  int row;
} game_Tile;

typedef enum { GAME_BOOT, GAME_TITLE, GAME_MENU, GAME_READY, GAME_RUN, GAME_PAUSE, GAME_OVER } game_GameState;

typedef struct {
  game_GameState state;
  game_GameState lastState;
  int            level;
#ifndef NDEBUG
  size_t fpsIndex;
#endif
} Game;

typedef struct game_Actor game_Actor;

// --- Constants ---

constexpr int ACTOR_SIZE       = 16;
constexpr int TILE_SIZE        = 16;
constexpr int CREATURE_COUNT   = 4;
constexpr int PLAYER_LIVES     = 3;
constexpr int PLAYER_MAX_LIVES = 10;

extern const Vector2 MAZE_ORIGIN;

extern const float BASE_SLOP;
extern const float BASE_DT;
extern const float MIN_SLOP;
extern const float MAX_SLOP;
extern const float OVERLAP_EPSILON;

extern const char* DIR_STRINGS[];

constexpr int WAIL_SOUND_COUNT = 4;

constexpr int MAX_KEY_TYPES = 2;

// --- Global state ---

extern log_Log* game_log;
extern Game     g_game;

// --- Helper functions ---

static inline bool game_isAABBColliding(game_AABB a, game_AABB b) {
  return a.min.x < b.max.x && a.max.x > b.min.x && a.min.y < b.max.y && a.max.y > b.min.y;
}

static inline float game_getAABBOverlapX(game_AABB a, game_AABB b) {
  return fmin(a.max.x, b.max.x) - fmax(a.min.x, b.min.x);
}

static inline float game_getAABBOverlapY(game_AABB a, game_AABB b) {
  return fmin(a.max.y, b.max.y) - fmax(a.min.y, b.min.y);
}

static inline void game_drawAABBOverlay(game_AABB aabb, Color colour) {
  Vector2 min = POS_ADJUST(aabb.min);
  Vector2 max = POS_ADJUST(aabb.max);
  engine_drawRectangleOutline((Rectangle) { min.x, min.y, max.x - min.x, max.y - min.y }, colour);
}

static inline game_Dir game_getOppositeDir(game_Dir dir) { return (dir + 2) % DIR_COUNT; }

// --- Internal game functions (game.c) ---

void game_new(void);
void game_over(void);
int  game_getLevel(void);
void game_nextLevel(void);
void game_playerDead(void);
