#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include "game_internal.h"

// --- Constants ---

const Vector2 VELS[] = {
    {0.0f, -1.0f},  // Up
    {1.0f, 0.0f},   // Right
    {0.0f, 1.0f},   // Down
    {-1.0f, 0.0f}   // Left
};

// --- Actor functions ---

AABB game__getActorAABB(Actor actor) {
  return (AABB) {.min = (Vector2) {actor.pos.x, actor.pos.y},
                 .max = (Vector2) {actor.pos.x + ACTOR_SIZE, actor.pos.y + ACTOR_SIZE}};
}
