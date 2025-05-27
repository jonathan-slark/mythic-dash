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
  AABB aabb;
  bool isWall;
  bool isCollision;
} Tile;

typedef struct Actor {
  Vector2 pos;
  Vector2 size;
  Dir     dir;
  float   speed;
  bool    isMoving;
  Tile    tilesMove[TILES_COUNT];
  Tile    tilesCanMove[TILES_COUNT];
  bool    isCanMove;
} Actor;

// --- Constants ---

static const Vector2 VELS[] = {
    {0.0f, 0.0f},   // None
    {0.0f, -1.0f},  // Up
    {1.0f, 0.0f},   // Right
    {0.0f, 1.0f},   // Down
    {-1.0f, 0.0f}   // Left
};

// --- Helper functions ---

/*
 * Resolves a collision between an actor and a static wall AABB.
 * Pushes the actor out along the axis of least penetration.
 */
static void resolveActorCollision(Actor* actor, const AABB* wall) {
  assert(actor != nullptr);
  assert(wall != nullptr);

  // Calculate how much the actor overlaps the wall on each axis
  float overlapX = fminf(actor->pos.x + actor->size.x, wall->max.x) - fmaxf(actor->pos.x, wall->min.x);
  float overlapY = fminf(actor->pos.y + actor->size.y, wall->max.y) - fmaxf(actor->pos.y, wall->min.y);

  // Choose the axis of minimum penetration to resolve the collision
  if (overlapX < overlapY) {
    // Handle horizontal (X-axis) collision
    if (actor->pos.x < wall->min.x) {
      // Actor is to the left of the wall: move it leftward out of the wall
      actor->pos.x -= overlapX;
    } else {
      // Actor is to the right of the wall: move it rightward out of the wall
      actor->pos.x += overlapX;
    }
  } else {
    // Handle vertical (Y-axis) collision
    if (actor->pos.y < wall->min.y) {
      // Actor is above the wall: move it upward out of the wall
      actor->pos.y -= overlapY;
    } else {
      // Actor is below the wall: move it downward out of the wall
      actor->pos.y += overlapY;
    }
  }
}

static Tile* isMazeCollision(Actor* actor, Dir dir) {
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

static void checkMazeCollision(Actor* actor) {
  assert(actor != nullptr);
  Tile* tile = isMazeCollision(actor, actor->dir);
  if (tile != nullptr) {
    resolveActorCollision(actor, &tile->aabb);
    LOG_DEBUG(game__log, "Collision detected, actor moved to: %f, %f", actor->pos.x, actor->pos.y);
    actor->isMoving = false;
  }
}

static void getWalls(Actor* actor, Tile tiles[], Dir dir) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE && dir < DIR_COUNT);

  // Base position calculation based on direction
  Vector2 basePos;
  Vector2 offset;

  switch (dir) {
    case DIR_UP:
      basePos = (Vector2) {actor->pos.x - TILE_SIZE * 0.5f, actor->pos.y - TILE_SIZE / 2.0f};
      offset  = (Vector2) {TILE_SIZE, 0};
      break;
    case DIR_RIGHT:
      basePos = (Vector2) {actor->pos.x + actor->size.x + TILE_SIZE / 2.0f, actor->pos.y - TILE_SIZE * 0.5f};
      offset  = (Vector2) {0, TILE_SIZE};
      break;
    case DIR_DOWN:
      basePos = (Vector2) {actor->pos.x - TILE_SIZE * 0.5f, actor->pos.y + actor->size.y + TILE_SIZE / 2.0f};
      offset  = (Vector2) {TILE_SIZE, 0};
      break;
    case DIR_LEFT:
      basePos = (Vector2) {actor->pos.x - TILE_SIZE / 2.0f, actor->pos.y - TILE_SIZE * 0.5f};
      offset  = (Vector2) {0, TILE_SIZE};
      break;
    default: assert(false); return;
  }

  // Generate and draw all four AABBs in a loop
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

static void alignToPassage(Actor* actor, Dir dir, float distance) {
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
// --- Actor functions ---

Actor* actor_create(Vector2 pos, Vector2 size, Dir dir, float speed) {
  assert(pos.x >= 0.0f);
  assert(pos.y >= 0.0f);
  assert(size.x > 0.0f);
  assert(size.y > 0.0f);
  assert(dir != DIR_NONE);
  assert(speed > 0.0f);

  Actor* actor = (Actor*) malloc(sizeof(Actor));
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

void actor_destroy(Actor** actor) {
  assert(actor != nullptr);
  free(*actor);
  *actor = nullptr;
}

Dir actor_getDir(const Actor* actor) {
  assert(actor != nullptr);
  return actor->dir;
}

Vector2 actor_getPos(const Actor* actor) {
  assert(actor != nullptr);
  return actor->pos;
}

Vector2 actor_getSize(const Actor* actor) {
  assert(actor != nullptr);
  return actor->size;
}

AABB actor_getAABB(const Actor* actor) {
  assert(actor != nullptr);
  return (AABB) {.min = (Vector2) {actor->pos.x, actor->pos.y},
                 .max = (Vector2) {actor->pos.x + actor->size.x, actor->pos.y + actor->size.x}};
}

bool actor_canMove(Actor* actor, Dir dir, float slop) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  actor->isCanMove = true;
  getWalls(actor, actor->tilesCanMove, dir);
  AABB actorAABB = actor_getAABB(actor);

  bool canMove   = false;
  Tile tile0     = actor->tilesCanMove[0];
  Tile tile1     = actor->tilesCanMove[1];
  Tile tile2     = actor->tilesCanMove[2];
  Tile tile3     = actor->tilesCanMove[3];
  if (tile0.isWall && !tile1.isWall && !tile2.isWall && tile3.isWall) {
    // Passage special case, allow actor to enter if close enough
    AABB aabb0 = tile0.aabb;
    AABB aabb3 = tile3.aabb;
    switch (dir) {
      case DIR_UP:
      case DIR_DOWN:
        float overlapX = aabb_getOverlapX(actorAABB, aabb0);
        float overlapY = aabb_getOverlapY(actorAABB, aabb0);
        if (overlapX > 0.0f && overlapX <= slop && overlapY == 0.0f) {
          alignToPassage(actor, dir, overlapX);
          LOG_DEBUG(game__log, "Actor can move up/down, moved actor to: %f, %f", actor->pos.x, actor->pos.y);
          actor->tilesCanMove[0].isCollision = false;
          actor->tilesCanMove[1].isCollision = false;
          actor->tilesCanMove[2].isCollision = false;
          actor->tilesCanMove[3].isCollision = false;
          canMove                            = true;
          break;
        }
        overlapX = aabb_getOverlapX(actorAABB, aabb3);
        overlapY = aabb_getOverlapY(actorAABB, aabb3);
        if (overlapX > 0.0f && overlapX <= slop && overlapY == 0.0f) {
          alignToPassage(actor, dir, -overlapX);
          LOG_DEBUG(game__log, "Actor can move up/down, moved actor to: %f, %f", actor->pos.x, actor->pos.y);
          actor->tilesCanMove[0].isCollision = false;
          actor->tilesCanMove[1].isCollision = false;
          actor->tilesCanMove[2].isCollision = false;
          actor->tilesCanMove[3].isCollision = false;
          canMove                            = true;
          break;
        }
        break;
      case DIR_LEFT:
      case DIR_RIGHT:
        overlapX = aabb_getOverlapX(actorAABB, aabb0);
        overlapY = aabb_getOverlapY(actorAABB, aabb0);
        if (overlapY > 0.0f && overlapY <= slop && overlapX == 0.0f) {
          alignToPassage(actor, dir, overlapY);
          LOG_DEBUG(game__log, "Actor can move left/right, moved actor to: %f, %f", actor->pos.x, actor->pos.y);
          actor->tilesCanMove[0].isCollision = false;
          actor->tilesCanMove[1].isCollision = false;
          actor->tilesCanMove[2].isCollision = false;
          actor->tilesCanMove[3].isCollision = false;
          canMove                            = true;
          break;
        }
        overlapX = aabb_getOverlapX(actorAABB, aabb3);
        overlapY = aabb_getOverlapY(actorAABB, aabb3);
        if (overlapY > 0.0f && overlapY <= slop && overlapX == 0.0f) {
          alignToPassage(actor, dir, -overlapY);
          LOG_DEBUG(game__log, "Actor can move left/right, moved actor to: %f, %f", actor->pos.x, actor->pos.y);
          actor->tilesCanMove[0].isCollision = false;
          actor->tilesCanMove[1].isCollision = false;
          actor->tilesCanMove[2].isCollision = false;
          actor->tilesCanMove[3].isCollision = false;
          canMove                            = true;
          break;
        }
        break;
      default: assert(false);
    }
  }
  if (canMove) {
    if (!actor->isMoving) {
      actor->isMoving = true;
    }
    return true;
  }

  // Usual method, strict checking
  canMove = true;
  for (size_t i = 0; i < TILES_COUNT; i++) {
    if (!actor->tilesCanMove[i].isWall) {
      actor->tilesCanMove[i].isCollision = false;
      continue;
    }

    AABB tileAABB = actor->tilesCanMove[i].aabb;
    switch (dir) {
      case DIR_UP:
      case DIR_DOWN:
        if (aabb_getOverlapX(actorAABB, tileAABB) > 0.0f && aabb_getOverlapY(actorAABB, tileAABB) == 0.0f) {
          actor->tilesCanMove[i].isCollision = true;
          canMove                            = false;
        } else {
          actor->tilesCanMove[i].isCollision = false;
        }
        break;
      case DIR_LEFT:
      case DIR_RIGHT:
        if (aabb_getOverlapY(actorAABB, tileAABB) > 0.0f && aabb_getOverlapX(actorAABB, tileAABB) == 0.0f) {
          actor->tilesCanMove[i].isCollision = true;
          canMove                            = false;
        } else {
          actor->tilesCanMove[i].isCollision = false;
        }
        break;
      default: assert(false);
    }
  }

  if (!actor->isMoving) {
    actor->isMoving = canMove;
  }
  return canMove;
}

void actor_overlay(const Actor* actor, Color colour) {
  assert(actor != nullptr);
  aabb_drawOverlay(actor_getAABB(actor), colour);
}

void actor_moveOverlay(Actor* actor) {
  assert(actor != nullptr);

  for (size_t i = 0; i < TILES_COUNT; i++) {
    drawTile(actor->tilesMove[i]);
  }
}

void actor_canMoveOverlay(Actor* actor) {
  assert(actor != nullptr);

  if (!actor->isCanMove) return;

  for (size_t i = 0; i < TILES_COUNT; i++) {
    drawTile(actor->tilesCanMove[i]);
  }
  actor->isCanMove = false;
}

void actor_move(Actor* actor, Dir dir, float frameTime) {
  assert(actor != nullptr);

  if (!actor->isMoving) return;

  actor->pos = Vector2Add(actor->pos, Vector2Scale(VELS[dir], frameTime * actor->speed));
  actor->dir = dir;

  getWalls(actor, actor->tilesMove, dir);
  checkMazeCollision(actor);
}
