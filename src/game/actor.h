#pragma once
// Clang format Language: C

#include "internal.h"
#include <stddef.h> // size_t

// --- Constants ---

constexpr size_t TILES_COUNT = 4;

// --- Types ---

typedef struct game__Tile {
  game__AABB aabb;
  bool isWall;
  bool isCollision;
} game__Tile;

typedef struct game__Actor {
  Vector2 pos;
  Vector2 size;
  game__Dir dir;
  float speed;
  bool isMoving;
  game__Tile tilesMove[TILES_COUNT];
  game__Tile tilesCanMove[TILES_COUNT];
  bool isCanMove;
} game__Actor;
