/*
 * Actor functions are split into G files: actor.c and actor_move.c with header actor.h.
 */

#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>  // malloc, free
#include "../internal.h"
#include "../maze/maze.h"
#include "internal.h"

// --- Constants ---

#define OVERLAY_COLOUR_TILE_WALL (Color){ 255, 100, 100, 128 }
#define OVERLAY_COLOUR_TILE_FLOOR (Color){ 100, 255, 100, 64 }
#define OVERLAY_COLOUR_COLLISION (Color){ 255, 255, 100, 128 }

// --- Helper functions ---

static void drawTile(actor__Tile tile) {
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

Vector2 actor_getCentre(const game__Actor* actor) {
  return (Vector2) { actor->pos.x + actor->size.x / 2.0f, actor->pos.y + actor->size.y / 2.0f };
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

float actor_getSpeed(game__Actor* actor) {
  assert(actor != nullptr);
  return actor->speed;
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

  for (game__Dir dir = 0; dir < DIR_COUNT; dir++) {
    if (!actor->isCanMove[dir]) continue;
    for (size_t i = 0; i < TILES_COUNT; i++) {
      // TODO: mark an array when when tile is drawn
      drawTile(actor->tilesCanMove[dir][i]);
    }
    actor->isCanMove[dir] = false;
  }
}

game__Tile actor_nextTile(game__Actor* actor, game__Dir dir) {
  Vector2 center = { actor->pos.x + actor->size.x / 2.0f, actor->pos.y + actor->size.y / 2.0f };
  switch (dir) {
    case DIR_UP: center.y -= TILE_SIZE; break;
    case DIR_RIGHT: center.x += TILE_SIZE; break;
    case DIR_DOWN: center.y += TILE_SIZE; break;
    case DIR_LEFT: center.x -= TILE_SIZE; break;
    default: assert(false);
  }
  return maze_getTile(center);
}

void actor_startMoving(game__Actor* actor) { actor->isMoving = true; }

bool actor_isColliding(const game__Actor* actor1, const game__Actor* actor2) {
  Vector2 centreActor1 = Vector2AddValue(actor1->pos, ACTOR_SIZE / 2.0f);
  Vector2 centreActor2 = Vector2AddValue(actor2->pos, ACTOR_SIZE / 2.0f);
  bool    isCollision  = Vector2Distance(centreActor1, centreActor2) < ACTOR_SIZE;
  if (isCollision)
    LOG_DEBUG(
        game__log,
        "Collision detected between actor1 (%f, %f) and actor2 (%f, %f)",
        actor1->pos.x,
        actor1->pos.y,
        actor2->pos.x,
        actor2->pos.y
    );
  return isCollision;
}
