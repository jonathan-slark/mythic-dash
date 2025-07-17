#include <assert.h>
#include <math.h>  // fminf, fmaxf
#include <raylib.h>
#include "../internal.h"
#include "../maze/maze.h"
#include "actor.h"
#include "internal.h"
#include "log/log.h"

// --- Constants ---

static const Vector2 VELS[] = {
  {  0.0f, -1.0f }, // Up
  {  1.0f,  0.0f }, // Right
  {  0.0f,  1.0f }, // Down
  { -1.0f,  0.0f }  // Left
};

const char* DIR_STRINGS[] = { "UP", "RIGHT", "DOWN", "LEFT" };

// --- Helper functions ---

/*
 * Resolves a collision between an actor and a static wall game_AABB.
 * Pushes the actor out along the axis of least penetration.
 */
static void resolveActorCollision(game_Actor* actor, const game_AABB* wall) {
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

static actor_Tile* isMazeCollision(game_Actor* actor) {
  assert(actor != nullptr);

  actor_Tile* tile = nullptr;
  for (size_t i = 0; i < TILES_COUNT; i++) {
    if (actor->tilesMove[i].isWall && game_isAABBColliding(actor_getAABB(actor), actor->tilesMove[i].aabb)) {
      actor->tilesMove[i].isCollision = true;
      tile                            = &actor->tilesMove[i];
    } else {
      actor->tilesMove[i].isCollision = false;
    }
  }
  return tile;
}

static void checkMazeCollision(game_Actor* actor) {
  assert(actor != nullptr);

  actor_Tile* tile = isMazeCollision(actor);
  if (tile != nullptr) {
    Vector2 oldPos = actor->pos;
    resolveActorCollision(actor, &tile->aabb);
    LOG_TRACE(
        game_log,
        "Collision detected, actor moved from: %f, %f to: %f, %f",
        oldPos.x,
        oldPos.y,
        actor->pos.x,
        actor->pos.y
    );
    actor->isMoving = false;
  }
}

static void getTiles(game_Actor* actor, actor_Tile tiles[], game_Dir dir) {
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
    tiles[i].isWall  = maze_isWall(position, actor->isPlayer);
  }
}

static void alignToPassage(game_Actor* actor, game_Dir dir, const game_AABB* tileAABB) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(tileAABB != nullptr);

  switch (dir) {
    case DIR_UP:
    case DIR_DOWN:
      // Align the actor's x-position to the tile edge for vertical movement.
      // If the actor is to the left of the tile, align its right edge to the tile's left edge.
      // Otherwise, align its left edge to the tile's right edge.
      if (actor->pos.x < tileAABB->min.x) {
        actor->pos.x = tileAABB->min.x - actor->size.x;
      } else {
        actor->pos.x = tileAABB->max.x;
      }
      break;

    case DIR_LEFT:
    case DIR_RIGHT:
      // Align the actor's y-position to the tile edge for horizontal movement.
      // If the actor is above the tile, align its bottom edge to the tile's top edge.
      // Otherwise, align its top edge to the tile's bottom edge.
      if (actor->pos.y < tileAABB->min.y) {
        actor->pos.y = tileAABB->min.y - actor->size.y;
      } else {
        actor->pos.y = tileAABB->max.y;
      }
      break;

    default: assert(false);
  }
}

static bool isPassagePattern(const actor_Tile tiles[]) {
  return !tiles[1].isWall && (tiles[0].isWall || tiles[2].isWall);
}

static void clearAllCollisionFlags(actor_Tile tiles[]) {
  for (size_t i = 0; i < TILES_COUNT; i++) {
    tiles[i].isCollision = false;
  }
}

static bool tryAlignToTile(game_Actor* actor, game_Dir dir, game_AABB actorAABB, game_AABB tileAABB, float slop) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  Vector2 oldPos   = actor->pos;
  float   overlapX = game_getAABBOverlapX(actorAABB, tileAABB);
  float   overlapY = game_getAABBOverlapY(actorAABB, tileAABB);
  switch (dir) {
    case DIR_UP:
    case DIR_DOWN:
      if (overlapX > OVERLAP_EPSILON && overlapX <= slop && fabsf(overlapY) < OVERLAP_EPSILON) {
        alignToPassage(actor, dir, &tileAABB);
        LOG_TRACE(
            game_log,
            "Actor can move %s, moved from %f, %f, to: %f, %f, slop: %f",
            DIR_STRINGS[dir],
            oldPos.x,
            oldPos.y,
            actor->pos.x,
            actor->pos.y,
            slop
        );
        return true;
      }
      break;
    case DIR_LEFT:
    case DIR_RIGHT:
      if (overlapY > OVERLAP_EPSILON && overlapY <= slop && fabsf(overlapX) < OVERLAP_EPSILON) {
        alignToPassage(actor, dir, &tileAABB);
        LOG_TRACE(
            game_log,
            "Actor can move %s, moved from %f, %f, to: %f, %f, slop: %f",
            DIR_STRINGS[dir],
            oldPos.x,
            oldPos.y,
            actor->pos.x,
            actor->pos.y,
            slop
        );
        return true;
      }
      break;
    default: assert(false);
  }
  return false;
}

static bool checkPassageMovement(game_Actor* actor, game_Dir dir, game_AABB actorAABB, float slop) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  if (!isPassagePattern(actor->tilesCanMove[dir])) {
    return false;
  }

  // Try aligning to tile0 first, then tile2
  if (tryAlignToTile(actor, dir, actorAABB, actor->tilesCanMove[dir][0].aabb, slop) ||
      tryAlignToTile(actor, dir, actorAABB, actor->tilesCanMove[dir][2].aabb, slop)) {
    clearAllCollisionFlags(actor->tilesCanMove[dir]);
    return true;
  }

  return false;
}

static bool checkStrictMovement(game_Actor* actor, game_Dir dir, game_AABB actorAABB) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  if (dir < 0 || dir >= DIR_COUNT) return false;

  bool canMove = true;

  for (size_t i = 0; i < TILES_COUNT; i++) {
    if (!actor->tilesCanMove[dir][i].isWall) {
      actor->tilesCanMove[dir][i].isCollision = false;
      continue;
    }

    game_AABB tileAABB     = actor->tilesCanMove[dir][i].aabb;
    bool      hasCollision = false;
    float     overlapX     = game_getAABBOverlapX(actorAABB, tileAABB);
    float     overlapY     = game_getAABBOverlapY(actorAABB, tileAABB);
    switch (dir) {
      case DIR_UP:
      case DIR_DOWN: hasCollision = (overlapX > OVERLAP_EPSILON && fabsf(overlapY) < OVERLAP_EPSILON); break;
      case DIR_LEFT:
      case DIR_RIGHT: hasCollision = (overlapY > OVERLAP_EPSILON && fabsf(overlapX) < OVERLAP_EPSILON); break;
      default: assert(false);
    }

    assert(dir >= 0 && dir < 4);
    assert(i < 3);
    actor->tilesCanMove[dir][i].isCollision = hasCollision;
    if (hasCollision) canMove = false;
  }

  return canMove;
}

static void checkTeleport(game_Actor* actor, game_Dir dir) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);

  Vector2 destPos;
  // Don't teleport till right on top of the tile
  if ((dir == DIR_UP && maze_isTeleport((Vector2) { actor->pos.x, actor->pos.y + actor->size.x - 1 }, &destPos)) ||
      (dir == DIR_RIGHT && maze_isTeleport(actor->pos, &destPos)) ||
      (dir == DIR_DOWN && maze_isTeleport(actor->pos, &destPos)) ||
      (dir == DIR_LEFT && maze_isTeleport((Vector2) { actor->pos.x + actor->size.x - 1, actor->pos.y }, &destPos))) {
    if (!actor->hasTeleported) {
      LOG_TRACE(
          game_log, "Teleporting actor from %.2f, %.2f to %.2f, %.2f", actor->pos.x, actor->pos.y, destPos.x, destPos.y
      );
      if (maze_reverseAfterTeleport()) {
        actor->dir = game_getOppositeDir(actor->dir);
        LOG_TRACE(game_log, "Reversing actor's direction");
      }
      actor->pos           = destPos;
      actor->hasTeleported = true;
    }
  } else {
    actor->hasTeleported = false;
  }
}

// --- Actor movement functions ---

bool actor_canMove(game_Actor* actor, game_Dir dir, float slop) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  actor->isCanMove[dir] = true;
  getTiles(actor, actor->tilesCanMove[dir], dir);
  game_AABB actorAABB = actor_getAABB(actor);

  // Try passage movement first, we are never perfectly lined up
  bool canMove = checkPassageMovement(actor, dir, actorAABB, slop);

  // If not a passage case, use strict collision checking
  if (!canMove) canMove = checkStrictMovement(actor, dir, actorAABB);

  // Update movement state if we can move
  if (!actor->isMoving && canMove) actor->isMoving = true;

  if (canMove) {
    LOG_TRACE(game_log, "Actor can move: %s, pos: %f, %f", DIR_STRINGS[dir], actor->pos.x, actor->pos.y);
  }

  return canMove;
}

void actor_moveNoCheck(game_Actor* actor, game_Dir dir, float frameTime) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(frameTime >= 0.0f);

  actor->pos = Vector2Add(actor->pos, Vector2Scale(VELS[dir], frameTime * actor->speed));
  actor->dir = dir;
}

void actor_move(game_Actor* actor, game_Dir dir, float frameTime) {
  assert(actor != nullptr);
  assert(dir >= 0 && dir < DIR_COUNT);
  assert(frameTime >= 0.0f);

  if (!actor->isMoving) return;

  actor_moveNoCheck(actor, dir, frameTime);

  // Teleport before collision check to avoid actor being stopped
  checkTeleport(actor, dir);
  getTiles(actor, actor->tilesMove, dir);
  checkMazeCollision(actor);
}

bool actor_hasTeleported(game_Actor* actor) {
  assert(actor != nullptr);
  return actor->hasTeleported;
}
