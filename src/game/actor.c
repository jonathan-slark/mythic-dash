#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include "game_internal.h"

// --- Constants ---

const int     ACTOR_SIZE = 16;

const Vector2 VELS[]     = {
    {0.0f, -1.0f},  // Up
    {1.0f, 0.0f},   // Right
    {0.0f, 1.0f},   // Down
    {-1.0f, 0.0f}   // Left
};

// --- Actor functions ---

bool game__actorCanMove(Actor actor, Dir dir, float distance) {
  assert(distance > 0.0f);

  Vector2 vel     = VELS[dir];
  Vector2 nextPos = Vector2Add(actor.pos, Vector2Scale(vel, distance));

  float   left    = nextPos.x;
  float   middleX = nextPos.x + ACTOR_SIZE / 2.0f;
  float   right   = nextPos.x + ACTOR_SIZE - 0.67f;
  float   top     = nextPos.y;
  float   middleY = nextPos.y + ACTOR_SIZE / 2.0f;
  float   bottom  = nextPos.y + ACTOR_SIZE - 0.67f;

  if (vel.x < 0) {
    return !(game__isMazeWall(left, top) || game__isMazeWall(left, middleY) || game__isMazeWall(left, bottom));
  }
  if (vel.x > 0) {
    return !(game__isMazeWall(right, top) || game__isMazeWall(right, middleY) || game__isMazeWall(right, bottom));
  }
  if (vel.y < 0) {
    return !(game__isMazeWall(left, top) || game__isMazeWall(middleX, top) || game__isMazeWall(right, top));
  }
  if (vel.y > 0) {
    return !(game__isMazeWall(left, bottom) || game__isMazeWall(middleX, bottom) || game__isMazeWall(right, bottom));
  }
  return true;
}
