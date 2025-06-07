/*
 * Actor functions are split into G files: actor.c and actor_move.c with header actor.h.
 */

#include "actor.h"
#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>  // malloc, free
#include "internal.h"

// --- Constants ---

#define OVERLAY_COLOUR_TILE_WALL (Color){ 255, 100, 100, 128 }
#define OVERLAY_COLOUR_TILE_FLOOR (Color){ 255, 100, 100, 32 }
#define OVERLAY_COLOUR_COLLISION (Color){ 255, 255, 100, 128 }

// --- Helper functions ---

static void drawTile(game__Tile tile) {
  Color colour;
  if (tile.isCollision) {
    colour = OVERLAY_COLOUR_COLLISION;
  } else {
    colour = tile.isWall ? OVERLAY_COLOUR_TILE_WALL : OVERLAY_COLOUR_TILE_FLOOR;
  }
  aabb_drawOverlay(tile.aabb, colour);
}

// --- Actor functions ---

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

Vector2 actor_getPos(const game__Actor* actor) {
  assert(actor != nullptr);
  return actor->pos;
}

void actor_setPos(game__Actor* actor, Vector2 pos) {
  assert(actor != nullptr);
  actor->pos = pos;
}

Vector2 actor_getSize(const game__Actor* actor) {
  assert(actor != nullptr);
  return actor->size;
}

game__Dir actor_getDir(const game__Actor* actor) {
  assert(actor != nullptr);
  return actor->dir;
}

void actor_setDir(game__Actor* actor, game__Dir dir) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  actor->dir = dir;
}

bool actor_isMoving(const game__Actor* actor) {
  assert(actor != nullptr);
  return actor->isMoving;
}

game__AABB actor_getAABB(const game__Actor* actor) {
  assert(actor != nullptr);
  return (game__AABB) {
    .min = (Vector2) {                 actor->pos.x,                 actor->pos.y },
    .max = (Vector2) { actor->pos.x + actor->size.x, actor->pos.y + actor->size.y }
  };
}

void actor_setSpeed(game__Actor* actor, float speed) {
  assert(actor != nullptr);
  assert(speed > 0.0f);
  actor->speed = speed;
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
