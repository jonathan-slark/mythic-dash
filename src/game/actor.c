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
    {0.0f, -1.0f},  // Up
    {1.0f, 0.0f},   // Right
    {0.0f, 1.0f},   // Down
    {-1.0f, 0.0f}   // Left
};

// --- Helper functions ---

static void resolveActorCollision(Actor* actor, AABB* wall) {
  assert(actor != nullptr);
  assert(wall != nullptr);

  float overlapX = fminf((actor->pos.x + actor->size.x), wall->max.x) - fmaxf(actor->pos.x, wall->min.x);
  float overlapY = fminf((actor->pos.y + actor->size.y), wall->max.y) - fmaxf(actor->pos.y, wall->min.y);

  if (overlapX < overlapY) {
    // X-axis collision (horizontal)
    if (actor->pos.x < wall->min.x) {
      // Actor hit the wall from the left side
      actor->pos.x -= overlapX;
    } else {
      // Actor hit the wall from the right side
      actor->pos.x += overlapX;
    }
  } else {
    // Y-axis collision (vertical)
    if (actor->pos.y < wall->min.y) {
      // Actor hit the wall from the top
      actor->pos.y -= overlapY;
    } else {
      // Actor hit the wall from the bottom
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
  assert(dir != None);
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
                 .max = (Vector2) {actor->pos.x + ACTOR_SIZE, actor->pos.y + ACTOR_SIZE}};
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
