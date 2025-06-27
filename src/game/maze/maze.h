/*
 * maze.h: Internal to maze units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../game.h"
#include <engine/engine.h>

constexpr int CHEST_SPAWN_COUNT = 2;

// --- Types ---

typedef enum maze__TileType {
  TILE_NONE,
  TILE_FLOOR,
  TILE_WALL,
  TILE_TELEPORT,
  TILE_COIN,
  TILE_SWORD,
  TILE_CHEST
} maze__TileType;

typedef struct maze__Tile {
  maze__TileType type;
  int linkedTeleportTile;
  bool isCoinCollected;
  bool isSwordCollected;
  bool isChestCollected;
  engine_Sprite *sprite;
  engine_Anim *anim;
  game__AABB aabb;
} maze__Tile;

typedef struct maze__Maze {
  int rows;
  int cols;
  int count;
  int tileWidth;
  int tileHeight;
  int layerCount;
  int coinCount;
  int chestID;
  bool hasChestSpawned[CHEST_SPAWN_COUNT];
  engine_Texture *tileset;
  maze__Tile *tiles;
} maze__Maze;

// --- Global state ---

extern maze__Maze g_maze;
