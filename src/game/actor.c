#include <assert.h>
#include <raylib.h>
#include "game_internal.h"

// --- Constants ---

const int ACTOR_SIZE = 16;

const Vector2 VELS[] = {
    {0.0f, -1.0f},  // Up
    {1.0f, 0.0f},   // Right
    {0.0f, 1.0f},   // Down
    {-1.0f, 0.0f}   // Left
};

// --- Actor functions ---

bool game__actorCanMove(Actor actor, float distance) {
  assert(actor.dir != None);
  assert(distance > 0.0f);

  Vector2 vel     = VELS[actor.dir];
  Vector2 nextPos = {actor.pos.x + vel.x * distance, actor.pos.y + vel.y * distance};

  float left      = nextPos.x;
  float right     = nextPos.x + ACTOR_SIZE - 1.0f;
  float top       = nextPos.y;
  float bottom    = nextPos.y + ACTOR_SIZE - 1.0f;

  if (vel.x < 0 && (game__isMazeWall(left, top) || game__isMazeWall(left, bottom))) {
    return false;
  }
  if (vel.x > 0 && (game__isMazeWall(right, top) || game__isMazeWall(right, bottom))) {
    return false;
  }
  if (vel.y < 0 && (game__isMazeWall(left, top) || game__isMazeWall(right, top))) {
    return false;
  }
  if (vel.y > 0 && (game__isMazeWall(left, bottom) || game__isMazeWall(right, bottom))) {
    return false;
  }
  return true;
}
