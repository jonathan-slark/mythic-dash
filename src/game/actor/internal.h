#pragma once
// Clang format Language: C

#include "../internal.h"
#include <stddef.h> // size_t

// --- Constants ---

constexpr size_t TILES_COUNT = 3;

// --- Types ---

typedef struct actor__Tile {
  game_AABB aabb;
  bool isWall;
  bool isCollision;
} actor__Tile;

typedef struct game_Actor {
  Vector2 pos;
  Vector2 size;
  game_Dir dir;
  float speed;
  bool isMoving;
  bool hasTeleported;
  actor__Tile tilesMove[TILES_COUNT];
  actor__Tile tilesCanMove[DIR_COUNT][TILES_COUNT];
  bool isCanMove[DIR_COUNT];
  bool isPlayer;
} game_Actor;
