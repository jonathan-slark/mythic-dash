#include <assert.h>
#include <math.h>  // fminf, fmaxf
#include "../game.h"
#include "actor.h"

// --- Constants ---

static const Vector2 VELS[] = {
  {  0.0f, -1.0f }, // Up
  {  1.0f,  0.0f }, // Right
  {  0.0f,  1.0f }, // Down
  { -1.0f,  0.0f }  // Left
};

const char* DIR_STRINGS[] = { "UP", "RIGHT", "DOWN", "LEFT" };
// const float MAZE_TELEPORTLEFT  = 8.0f;
// const float MAZE_TELEPORTRIGHT = 256.0f;

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

static game__Tile* isMazeCollision(game__Actor* actor) {
  assert(actor != nullptr);

  game__Tile* tile = nullptr;
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

  game__Tile* tile = isMazeCollision(actor);
  if (tile != nullptr) {
    resolveActorCollision(actor, &tile->aabb);
    LOG_DEBUG(game__log, "Collision detected, actor moved to: %f, %f", actor->pos.x, actor->pos.y);
    actor->isMoving = false;
  }
}

static void getTiles(game__Actor* actor, game__Tile tiles[], game__Dir dir) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);

  Vector2 basePos;
  Vector2 offset;

  switch (dir) {
    case DIR_UP:
      basePos = (Vector2) { actor->pos.x - TILE_SIZE / 2.0f, actor->pos.y - TILE_SIZE / 2.0f };
      offset  = (Vector2) { TILE_SIZE, 0 };
      break;
    case DIR_RIGHT:
      basePos = (Vector2) { actor->pos.x + actor->size.x + TILE_SIZE / 2.0f, actor->pos.y - TILE_SIZE / 2.0f };
      offset  = (Vector2) { 0, TILE_SIZE };
      break;
    case DIR_DOWN:
      basePos = (Vector2) { actor->pos.x - TILE_SIZE / 2.0f, actor->pos.y + actor->size.y + TILE_SIZE / 2.0f };
      offset  = (Vector2) { TILE_SIZE, 0 };
      break;
    case DIR_LEFT:
      basePos = (Vector2) { actor->pos.x - TILE_SIZE / 2.0f, actor->pos.y - TILE_SIZE / 2.0f };
      offset  = (Vector2) { 0, TILE_SIZE };
      break;
    default: assert(false); return;
  }

  for (size_t i = 0; i < TILES_COUNT; i++) {
    Vector2 position = Vector2Add(basePos, Vector2Scale(offset, i));
    tiles[i].aabb    = maze_getAABB(position);
    tiles[i].isWall  = maze_isWall(position);
  }
}

static void alignToPassage(game__Actor* actor, game__Dir dir, float distance) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(distance != 0.0f && distance < MAX_SLOP);

  switch (dir) {
    case DIR_UP:
    case DIR_DOWN: actor->pos.x += distance; break;
    case DIR_LEFT:
    case DIR_RIGHT: actor->pos.y += distance; break;
    default: assert(false);
  }
}

static bool isPassagePattern(const game__Tile tiles[]) {
  return tiles[0].isWall && !tiles[1].isWall && tiles[2].isWall;
}

static void clearAllCollisionFlags(game__Tile tiles[]) {
  for (size_t i = 0; i < TILES_COUNT; i++) {
    tiles[i].isCollision = false;
  }
}

static bool tryAlignToTile(
    game__Actor* actor,
    game__Dir    dir,
    game__AABB   actorAABB,
    game__AABB   tileAABB,
    float        slop,
    bool         isPositive
) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  float overlapX = aabb_getOverlapX(actorAABB, tileAABB);
  float overlapY = aabb_getOverlapY(actorAABB, tileAABB);
  switch (dir) {
    case DIR_UP:
    case DIR_DOWN:
      if (overlapX > OVERLAP_EPSILON && overlapX <= slop && fabsf(overlapY) < OVERLAP_EPSILON) {
        alignToPassage(actor, dir, isPositive ? overlapX : -overlapX);
        LOG_TRACE(
            game__log, "Actor can move up/down, actor moved to: %f, %f, slop: %f", actor->pos.x, actor->pos.y, slop
        );
        return true;
      }
      break;
    case DIR_LEFT:
    case DIR_RIGHT:
      if (overlapY > OVERLAP_EPSILON && overlapY <= slop && fabsf(overlapX) < OVERLAP_EPSILON) {
        alignToPassage(actor, dir, isPositive ? overlapY : -overlapY);
        LOG_TRACE(
            game__log, "Actor can move left/right, actor moved to: %f, %f, slop: %f", actor->pos.x, actor->pos.y, slop
        );
        return true;
      }
      break;
    default: assert(false);
  }
  return false;
}

static bool checkPassageMovement(game__Actor* actor, game__Dir dir, game__AABB actorAABB, float slop) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  if (!isPassagePattern(actor->tilesCanMove[dir])) {
    return false;
  }

  // Try aligning to tile0 first, then tile2
  if (tryAlignToTile(actor, dir, actorAABB, actor->tilesCanMove[dir][0].aabb, slop, true) ||
      tryAlignToTile(actor, dir, actorAABB, actor->tilesCanMove[dir][2].aabb, slop, false)) {
    clearAllCollisionFlags(actor->tilesCanMove[dir]);
    return true;
  }

  return false;
}

static bool checkStrictMovement(game__Actor* actor, game__Dir dir, game__AABB actorAABB) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);

  bool canMove = true;

  for (size_t i = 0; i < TILES_COUNT; i++) {
    if (!actor->tilesCanMove[dir][i].isWall) {
      actor->tilesCanMove[dir][i].isCollision = false;
      continue;
    }

    game__AABB tileAABB     = actor->tilesCanMove[dir][i].aabb;
    bool       hasCollision = false;
    float      overlapX     = aabb_getOverlapX(actorAABB, tileAABB);
    float      overlapY     = aabb_getOverlapY(actorAABB, tileAABB);
    switch (dir) {
      case DIR_UP:
      case DIR_DOWN: hasCollision = (overlapX > OVERLAP_EPSILON && fabsf(overlapY) < OVERLAP_EPSILON); break;
      case DIR_LEFT:
      case DIR_RIGHT: hasCollision = (overlapY > OVERLAP_EPSILON && fabsf(overlapX) < OVERLAP_EPSILON); break;
      default: assert(false);
    }

    actor->tilesCanMove[dir][i].isCollision = hasCollision;
    if (hasCollision) canMove = false;
  }

  return canMove;
}

// --- Actor movement functions ---

bool actor_canMove(game__Actor* actor, game__Dir dir, float slop) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  actor->isCanMove[dir] = true;
  getTiles(actor, actor->tilesCanMove[dir], dir);
  game__AABB actorAABB = actor_getAABB(actor);

  // Try passage movement first (special case for narrow passages)
  bool canMove = checkPassageMovement(actor, dir, actorAABB, slop);

  // If not a passage case, use strict collision checking
  if (!canMove) canMove = checkStrictMovement(actor, dir, actorAABB);

  // Update movement state if we can move
  if (!actor->isMoving && canMove) actor->isMoving = true;

  if (canMove) {
    LOG_TRACE(game__log, "Actor can move in direction: %s, pos: %f, %f", DIR_STRINGS[dir], actor->pos.x, actor->pos.y);
  }

  return canMove;
}

void actor_moveNoCheck(game__Actor* actor, game__Dir dir, float frameTime) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(frameTime >= 0.0f);

  actor->pos = Vector2Add(actor->pos, Vector2Scale(VELS[dir], frameTime * actor->speed));
  actor->dir = dir;

  // if (actor->pos.x < MAZE_TELEPORTLEFT) actor->pos.x = MAZE_TELEPORTRIGHT;
  // if (actor->pos.x > MAZE_TELEPORTRIGHT) actor->pos.x = MAZE_TELEPORTLEFT;
}

void actor_move(game__Actor* actor, game__Dir dir, float frameTime) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(frameTime >= 0.0f);

  if (!actor->isMoving) return;

  actor_moveNoCheck(actor, dir, frameTime);

  getTiles(actor, actor->tilesMove, dir);
  checkMazeCollision(actor);
}
