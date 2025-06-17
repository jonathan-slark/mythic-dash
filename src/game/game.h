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
#define POS_ADJUST(pos) Vector2Add((pos), MAZE_ORIGIN)

// --- Types ---

typedef enum game__Dir { DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_COUNT, DIR_NONE } game__Dir;

typedef struct game__AABB {
  Vector2 min;
  Vector2 max;
} game__AABB;

typedef struct game__Tile {
  int row;
  int col;
} game__Tile;

typedef struct game__Actor game__Actor;

// --- Constants ---

#define ASSET_DIR "../../asset/"
constexpr int ACTOR_SIZE     = 16;
constexpr int TILE_SIZE      = 16;
constexpr int CREATURE_COUNT = 4;

extern const Vector2 MAZE_ORIGIN;

extern const float BASE_SLOP;
extern const float BASE_DT;
extern const float MIN_SLOP;
extern const float MAX_SLOP;
extern const float OVERLAP_EPSILON;

extern const char* DIR_STRINGS[];

// --- Global state ---

extern log_Log* game__log;

// --- Helper functions ---

static inline bool aabb_isColliding(game__AABB a, game__AABB b) {
  return a.min.x < b.max.x && a.max.x > b.min.x && a.min.y < b.max.y && a.max.y > b.min.y;
}

static inline float aabb_getOverlapX(game__AABB a, game__AABB b) {
  return fmin(a.max.x, b.max.x) - fmax(a.min.x, b.min.x);
}

static inline float aabb_getOverlapY(game__AABB a, game__AABB b) {
  return fmin(a.max.y, b.max.y) - fmax(a.min.y, b.min.y);
}

static inline void aabb_drawOverlay(game__AABB aabb, Color colour) {
  Vector2 min = POS_ADJUST(aabb.min);
  Vector2 max = POS_ADJUST(aabb.max);
  engine_drawRectangleOutline((Rectangle) { min.x, min.y, max.x - min.x, max.y - min.y }, colour);
}

// --- Actor functions (actor.c) ---

game__Actor* actor_create(Vector2 pos, Vector2 size, game__Dir dir, float speed);
void         actor_destroy(game__Actor** actor);
Vector2      actor_getPos(const game__Actor* actor);
void         actor_setPos(game__Actor* actor, Vector2 pos);
Vector2      actor_getSize(const game__Actor* actor);
game__Dir    actor_getDir(const game__Actor* actor);
void         actor_setDir(game__Actor* actor, game__Dir dir);
bool         actor_isMoving(const game__Actor* actor);
game__AABB   actor_getAABB(const game__Actor* actor);
bool         actor_canMove(game__Actor* actor, game__Dir dir, float slop);
void         actor_setSpeed(game__Actor* actor, float speed);
void         actor_overlay(const game__Actor* actor, Color colour);
void         actor_moveOverlay(game__Actor* actor);
void         actor_canMoveOverlay(game__Actor* actor);
void         actor_moveNoCheck(game__Actor* actor, game__Dir dir, float frameTime);
void         actor_move(game__Actor* actor, game__Dir dir, float frameTime);
void         actor_update(game__Actor* actor, float frameTime);
game__Tile   actor_nextTile(game__Actor* actor, game__Dir dir);

// --- Player functions (player.c) ---

bool         player_init(void);
void         player_shutdown(void);
void         player_update(float frameTime, float slop);
Vector2      player_getPos(void);
game__Dir    player_getDir(void);
bool         player_isMoving(void);
game__Actor* player_getActor(void);
game__Tile   player_tileAhead(int tileNum);

// --- Ghost functions (ghost.c) ---

bool         ghost_init(void);
void         ghost_shutdown(void);
void         ghost_update(float frameTime, float slop);
Vector2      ghost_getPos(int id);
game__Dir    ghost_getDir(int id);
game__Actor* ghost_getActor(int id);
float        ghost_getDecisionCooldown(int id);
const char*  ghost_getStateString(int id);
game__Tile   ghost_getTarget(int id);

// --- Maze functions (maze.c) ---

[[nodiscard]] bool maze_init(void);
void               maze_shutdown(void);
game__AABB         maze_getAABB(Vector2 pos);
bool               maze_isWall(Vector2 pos);
bool               maze_isTeleport(Vector2 pos, Vector2* dest);
void               maze_tilesOverlay(void);
void               maze_draw(void);
void               maze_update(float frameTime);
game__Tile         maze_getTile(Vector2 pos);
Vector2            maze_getPos(game__Tile tile);
int                maze_manhattanDistance(game__Tile nextTile, game__Tile targetTile);
game__Tile         maze_doubleVectorBetween(game__Tile from, game__Tile to);

// --- Debug functions (debug.c) ---

void debug_drawOverlay(void);
void debug_toggleFPSOverlay(void);
void debug_toggleMazeOverlay(void);
void debug_togglePlayerOverlay(void);
void debug_toggleGhostOverlay(void);
