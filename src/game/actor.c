#include <assert.h>
#include <math.h>  // fminf, fmaxf
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>  // malloc, free
#include "internal.h"

// --- Constants ---

constexpr size_t WALLS_COUNT = 4;

// --- Types ---

typedef struct Actor {
  Vector2 pos;
  Vector2 size;
  Dir     dir;
  float   speed;
  AABB    walls[WALLS_COUNT];        // AABB of the tiles in the direction of the actor
  bool    isWall[WALLS_COUNT];       // Whether the tile is a wall
  bool    isCollision[WALLS_COUNT];  // Whether the actor is colliding with the wall
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

  // LOG_TRACE(game__log, "Resolved collision: %f, %f", actor->pos.x, actor->pos.y);
}

static const AABB* isMazeCollision(Actor* actor, Dir dir) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE);

  AABB* wall = nullptr;
  for (size_t i = 0; i < WALLS_COUNT; i++) {
    if (actor->isWall[i] && aabb_isColliding(actor_getAABB(actor), actor->walls[i])) {
      actor->isCollision[i] = true;
      wall                  = &actor->walls[i];
    } else {
      actor->isCollision[i] = false;
    }
  }
  return wall;
}

static void checkMazeCollision(Actor* actor) {
  assert(actor != nullptr);
  const AABB* wall = isMazeCollision(actor, actor->dir);
  if (wall != nullptr) {
    resolveActorCollision(actor, wall);
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

  actor->pos   = pos;
  actor->size  = size;
  actor->dir   = dir;
  actor->speed = speed;

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

bool actor_canMove(Actor* actor, Dir dir) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE);

  actor_getWalls(actor, dir);
  AABB actorAABB = actor_getAABB(actor);
  bool canMove   = true;
  for (size_t i = 0; i < WALLS_COUNT; i++) {
    if (!actor->isWall[i]) {
      actor->isCollision[i] = false;
      continue;
    }

    AABB wallAABB = actor->walls[i];
    switch (dir) {
      case DIR_UP:
      case DIR_DOWN:
        if (aabb_getOverlapX(actorAABB, wallAABB) > 0.0f && aabb_getOverlapY(actorAABB, wallAABB) == 0.0f) {
          actor->isCollision[i] = true;
          canMove               = false;
        } else {
          actor->isCollision[i] = false;
        }
        break;
      case DIR_LEFT:
      case DIR_RIGHT:
        if (aabb_getOverlapY(actorAABB, wallAABB) > 0.0f && aabb_getOverlapX(actorAABB, wallAABB) == 0.0f) {
          actor->isCollision[i] = true;
          canMove               = false;
        } else {
          actor->isCollision[i] = false;
        }
        break;
      default: assert(false);
    }
  }
  return canMove;
}

void actor_getWalls(Actor* actor, Dir dir) {
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
  for (size_t i = 0; i < WALLS_COUNT; i++) {
    Vector2 position = Vector2Add(basePos, Vector2Scale(offset, i));
    actor->walls[i]  = maze_getAABB(position);
    actor->isWall[i] = maze_isWall(position);
  }
}

void actor_overlay(const Actor* actor, Color colour) {
  assert(actor != nullptr);
  aabb_drawOverlay(actor_getAABB(actor), colour);
}

#ifndef NDEBUG
void actor_wallsOverlay(const Actor* actor) {
  assert(actor != nullptr);

  for (size_t i = 0; i < WALLS_COUNT; i++) {
    Color colour;
    if (actor->isCollision[i]) {
      colour = OVERLAY_COLOUR_COLLISION;
    } else {
      colour = actor->isWall[i] ? OVERLAY_COLOUR_TILE_WALL : OVERLAY_COLOUR_TILE_FLOOR;
    }
    aabb_drawOverlay(actor->walls[i], colour);
  }
}
#endif

void actor_move(Actor* actor, Dir dir, float frameTime) {
  assert(actor != nullptr);
  actor->pos = Vector2Add(actor->pos, Vector2Scale(VELS[dir], frameTime * actor->speed));
  actor->dir = dir;
  // LOG_TRACE(game__log, "Actor moved to: %f, %f", actor->pos.x, actor->pos.y);

  actor_getWalls(actor, dir);
  checkMazeCollision(actor);
}
