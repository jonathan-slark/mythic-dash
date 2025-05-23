#include <assert.h>
#include <math.h>  // fminf, fmaxf
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>  // malloc, free
#include "internal.h"

// --- Types ---

typedef struct Actor {
  Vector2 pos;
  Vector2 size;
  Dir     dir;
  float   speed;
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
static void resolveActorCollision(Actor* actor, AABB* wall) {
  assert(actor != nullptr);
  assert(wall != nullptr);

  // Calculate how much the actor overlaps the wall on the X axis
  float overlapX = fminf(actor->pos.x + actor->size.x, wall->max.x) - fmaxf(actor->pos.x, wall->min.x);

  // Calculate how much the actor overlaps the wall on the Y axis
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

bool actor_canMove(const Actor* actor, Dir dir) {
  assert(actor != nullptr);
  assert(dir != DIR_NONE);

  Vector2 vel  = VELS[dir];
  AABB    aabb = {.min = Vector2Add(actor->pos, vel), .max = Vector2Add(Vector2Add(actor->pos, actor->size), vel)};
  return maze_isHittingWall(aabb) == nullptr;
}

void actor_move(Actor* actor, Dir dir, float frameTime) {
  assert(actor != nullptr);
  actor->pos = Vector2Add(actor->pos, Vector2Scale(VELS[dir], frameTime * actor->speed));
  actor->dir = dir;
}

void actor_checkMazeCollision(Actor* actor) {
  assert(actor != nullptr);
  AABB* wall = maze_isHittingWall(actor_getAABB(actor));
  if (wall != nullptr) {
    resolveActorCollision(actor, wall);
  }
}
