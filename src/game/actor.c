#include <assert.h>
#include <math.h>  // fminf, fmaxf
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>  // malloc, free
#include "internal.h"

// --- Constants ---

constexpr size_t TILES_COUNT = 4;

// --- Types ---

typedef struct Tile {
  game__AABB aabb;
  bool       isWall;
  bool       isCollision;
} Tile;

typedef struct game__Actor {
  Vector2   pos;
  Vector2   size;
  game__Dir dir;
  float     speed;
  bool      isMoving;
  Tile      tilesMove[TILES_COUNT];
  Tile      tilesCanMove[TILES_COUNT];
  bool      isCanMove;
} game__Actor;

// --- Constants ---

static const Vector2 VELS[] = {
    {0.0f, 0.0f},   // None
    {0.0f, -1.0f},  // Up
    {1.0f, 0.0f},   // Right
    {0.0f, 1.0f},   // Down
    {-1.0f, 0.0f}   // Left
};

static const char* DIR_STRINGS[] = {"NONE", "UP", "RIGHT", "DOWN", "LEFT"};

// --- Helper functions ---

/*
 * Resolves a collision between an actor and a static wall game__AABB.
 * Pushes the actor out along the axis of least penetration.
 */
static void resolveActorCollision(game__Actor* actor, const game__AABB* wall) {
  assert(actor != nullptr);
  assert(wall != nullptr);

  // Calculate how much the actor overlaps the wall on each axis
  float overlapX = fminf(actor->pos.x + actor->size.x, wall->max.x) - fmaxf(actor->pos.x, wall->min.x);
  float overlapY = fminf(actor->pos.y + actor->size.y, wall->max.y) - fmaxf(actor->pos.y, wall->min.y);

  // Choose the axis of minimum penetration to resolve the collision
  if (overlapX < overlapY) {
    // Handle horizontal (X-axis) collision
    if (actor->pos.x < wall->min.x) {
      // game__Actor is to the left of the wall: move it leftward out of the wall
      actor->pos.x -= overlapX;
    } else {
      // game__Actor is to the right of the wall: move it rightward out of the wall
      actor->pos.x += overlapX;
    }
  } else {
    // Handle vertical (Y-axis) collision
    if (actor->pos.y < wall->min.y) {
      // game__Actor is above the wall: move it upward out of the wall
      actor->pos.y -= overlapY;
    } else {
      // game__Actor is below the wall: move it downward out of the wall
      actor->pos.y += overlapY;
    }
  }
}

static Tile* isMazeCollision(game__Actor* actor, game__Dir dir) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE);

  Tile* tile = nullptr;
  for (size_t i = 0; i < TILES_COUNT; i++) {
    if (actor->tilesMove[i].isWall && aabb_isColliding(actor_getAABB(actor), actor->tilesMove[i].aabb)) {
      actor->tilesMove[i].isCollision = true;
      tile                            = &actor->tilesMove[i];
    } else {
      actor->tilesMove[i].isCollision = false;
    }
  }
  return tile;
}

static void checkMazeCollision(game__Actor* actor) {
  assert(actor != nullptr);
  Tile* tile = isMazeCollision(actor, actor->dir);
  if (tile != nullptr) {
    resolveActorCollision(actor, &tile->aabb);
    LOG_DEBUG(game__log, "Collision detected, actor moved to: %f, %f", actor->pos.x, actor->pos.y);
    actor->isMoving = false;
  }
}

static void getWalls(game__Actor* actor, Tile tiles[], game__Dir dir) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE && dir < DIR_COUNT);

  Vector2 basePos;
  Vector2 offset;

  switch (dir) {
    case DIR_UP:
      basePos = (Vector2) {actor->pos.x - TILE_SIZE / 2.0f, actor->pos.y - TILE_SIZE / 2.0f};
      offset  = (Vector2) {TILE_SIZE, 0};
      break;
    case DIR_RIGHT:
      basePos = (Vector2) {actor->pos.x + actor->size.x + TILE_SIZE / 2.0f, actor->pos.y - TILE_SIZE / 2.0f};
      offset  = (Vector2) {0, TILE_SIZE};
      break;
    case DIR_DOWN:
      basePos = (Vector2) {actor->pos.x - TILE_SIZE / 2.0f, actor->pos.y + actor->size.y + TILE_SIZE / 2.0f};
      offset  = (Vector2) {TILE_SIZE, 0};
      break;
    case DIR_LEFT:
      basePos = (Vector2) {actor->pos.x - TILE_SIZE / 2.0f, actor->pos.y - TILE_SIZE / 2.0f};
      offset  = (Vector2) {0, TILE_SIZE};
      break;
    default: assert(false); return;
  }

  for (size_t i = 0; i < TILES_COUNT; i++) {
    Vector2 position = Vector2Add(basePos, Vector2Scale(offset, i));
    tiles[i].aabb    = maze_getAABB(position);
    tiles[i].isWall  = maze_isWall(position);
  }
}

static void drawTile(Tile tile) {
  Color colour;
  if (tile.isCollision) {
    colour = OVERLAY_COLOUR_COLLISION;
  } else {
    colour = tile.isWall ? OVERLAY_COLOUR_TILE_WALL : OVERLAY_COLOUR_TILE_FLOOR;
  }
  aabb_drawOverlay(tile.aabb, colour);
}

static void alignToPassage(game__Actor* actor, game__Dir dir, float distance) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE);
  assert(distance != 0.0f && distance < MAX_SLOP);

  switch (dir) {
    case DIR_UP:
    case DIR_DOWN: actor->pos.x += distance; break;
    case DIR_LEFT:
    case DIR_RIGHT: actor->pos.y += distance; break;
    default: assert(false);
  }
}

static bool isPassagePattern(const Tile tiles[]) {
  return tiles[0].isWall && !tiles[1].isWall && !tiles[2].isWall && tiles[3].isWall;
}

static void clearAllCollisionFlags(Tile tiles[]) {
  for (size_t i = 0; i < TILES_COUNT; i++) {
    tiles[i].isCollision = false;
  }
}

static bool tryAlignToTile(game__Actor* actor,
                           game__Dir    dir,
                           game__AABB   actorAABB,
                           game__AABB   tileAABB,
                           float        slop,
                           bool         isPositive) {
  float overlapX = aabb_getOverlapX(actorAABB, tileAABB);
  float overlapY = aabb_getOverlapY(actorAABB, tileAABB);
  switch (dir) {
    case DIR_UP:
    case DIR_DOWN:
      if (overlapX > EPSILON && overlapX <= slop && fabsf(overlapY) < EPSILON) {
        alignToPassage(actor, dir, isPositive ? overlapX : -overlapX);
        LOG_DEBUG(game__log, "Actor can move up/down, actor moved to: %f, %f", actor->pos.x, actor->pos.y);
        return true;
      }
      break;
    case DIR_LEFT:
    case DIR_RIGHT:
      if (overlapY > EPSILON && overlapY <= slop && fabsf(overlapX) < EPSILON) {
        alignToPassage(actor, dir, isPositive ? overlapY : -overlapY);
        LOG_DEBUG(game__log, "Actor can move left/right, actor moved to: %f, %f", actor->pos.x, actor->pos.y);
        return true;
      }
      break;
    default: assert(false);
  }
  return false;
}

static bool checkPassageMovement(game__Actor* actor, game__Dir dir, game__AABB actorAABB, float slop) {
  if (!isPassagePattern(actor->tilesCanMove)) {
    return false;
  }

  // Try aligning to tile0 first, then tile3
  if (tryAlignToTile(actor, dir, actorAABB, actor->tilesCanMove[0].aabb, slop, true) ||
      tryAlignToTile(actor, dir, actorAABB, actor->tilesCanMove[3].aabb, slop, false)) {
    clearAllCollisionFlags(actor->tilesCanMove);
    return true;
  }

  return false;
}

static bool checkStrictMovement(game__Actor* actor, game__Dir dir, game__AABB actorAABB) {
  bool canMove = true;

  for (size_t i = 0; i < TILES_COUNT; i++) {
    if (!actor->tilesCanMove[i].isWall) {
      actor->tilesCanMove[i].isCollision = false;
      continue;
    }

    game__AABB tileAABB     = actor->tilesCanMove[i].aabb;
    bool       hasCollision = false;
    float      overlapX     = aabb_getOverlapX(actorAABB, tileAABB);
    float      overlapY     = aabb_getOverlapY(actorAABB, tileAABB);
    switch (dir) {
      case DIR_UP:
      case DIR_DOWN: hasCollision = (overlapX > EPSILON && fabsf(overlapY) < EPSILON); break;
      case DIR_LEFT:
      case DIR_RIGHT: hasCollision = (overlapY > EPSILON && fabsf(overlapX) < EPSILON); break;
      default: assert(false);
    }

    actor->tilesCanMove[i].isCollision = hasCollision;
    if (hasCollision) {
      canMove = false;
    }
  }

  return canMove;
}

// --- game__Actor functions ---

game__Actor* actor_create(Vector2 pos, Vector2 size, game__Dir dir, float speed) {
  assert(pos.x >= 0.0f);
  assert(pos.y >= 0.0f);
  assert(size.x > 0.0f);
  assert(size.y > 0.0f);
  assert(dir != DIR_NONE);
  assert(speed > 0.0f);

  game__Actor* actor = (game__Actor*) malloc(sizeof(game__Actor));
  if (actor == nullptr) {
    LOG_ERROR(game__log, "Failed to create actor");
    return nullptr;
  }

  actor->pos      = pos;
  actor->size     = size;
  actor->dir      = dir;
  actor->speed    = speed;
  actor->isMoving = true;

  return actor;
}

void actor_destroy(game__Actor** actor) {
  assert(actor != nullptr);
  free(*actor);
  *actor = nullptr;
}

game__Dir actor_getDir(const game__Actor* actor) {
  assert(actor != nullptr);
  return actor->dir;
}

Vector2 actor_getPos(const game__Actor* actor) {
  assert(actor != nullptr);
  return actor->pos;
}

Vector2 actor_getSize(const game__Actor* actor) {
  assert(actor != nullptr);
  return actor->size;
}

game__AABB actor_getAABB(const game__Actor* actor) {
  assert(actor != nullptr);
  return (game__AABB) {.min = (Vector2) {actor->pos.x, actor->pos.y},
                       .max = (Vector2) {actor->pos.x + actor->size.x, actor->pos.y + actor->size.y}};
}

bool actor_canMove(game__Actor* actor, game__Dir dir, float slop) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  actor->isCanMove = true;
  getWalls(actor, actor->tilesCanMove, dir);
  game__AABB actorAABB = actor_getAABB(actor);

  // Try passage movement first (special case for narrow passages)
  bool canMove = checkPassageMovement(actor, dir, actorAABB, slop);

  // If not a passage case, use strict collision checking
  if (!canMove) canMove = checkStrictMovement(actor, dir, actorAABB);

  // Update movement state if we can move
  if (!actor->isMoving && canMove) actor->isMoving = true;

  if (canMove) {
    LOG_DEBUG(game__log, "Actor can move in direction: %s, pos: %f, %f", DIR_STRINGS[dir], actor->pos.x, actor->pos.y);
  }

  return canMove;
}

void actor_overlay(const game__Actor* actor, Color colour) {
  assert(actor != nullptr);
  aabb_drawOverlay(actor_getAABB(actor), colour);
}

void actor_moveOverlay(game__Actor* actor) {
  assert(actor != nullptr);

  for (size_t i = 0; i < TILES_COUNT; i++) {
    drawTile(actor->tilesMove[i]);
  }
}

void actor_canMoveOverlay(game__Actor* actor) {
  assert(actor != nullptr);

  if (!actor->isCanMove) return;

  for (size_t i = 0; i < TILES_COUNT; i++) {
    drawTile(actor->tilesCanMove[i]);
  }
  actor->isCanMove = false;
}

void actor_move(game__Actor* actor, game__Dir dir, float frameTime) {
  assert(actor != nullptr);

  if (!actor->isMoving) return;

  actor->pos = Vector2Add(actor->pos, Vector2Scale(VELS[dir], frameTime * actor->speed));
  actor->dir = dir;

  getWalls(actor, actor->tilesMove, dir);
  checkMazeCollision(actor);
}
