/*
 * actor.h: Internal to actor units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../game.h"
#include <stddef.h> // size_t

// --- Constants ---

constexpr size_t TILES_COUNT = 3;

// --- Types ---

typedef struct actor__Tile {
  game__AABB aabb;
  bool isWall;
  bool isCollision;
} actor__Tile;

typedef struct game__Actor {
  Vector2 pos;
  Vector2 size;
  game__Dir dir;
  float speed;
  bool isMoving;
  bool hasTeleported;
  actor__Tile tilesMove[TILES_COUNT];
  actor__Tile tilesCanMove[DIR_COUNT][TILES_COUNT];
  bool isCanMove[DIR_COUNT];
} game__Actor;
