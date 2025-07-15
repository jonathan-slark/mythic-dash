/*
 * maze.h: Internal to maze units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../internal.h"
#include <engine/engine.h>

constexpr int CHEST_SPAWN_COUNT = 2;

// --- Types ---

typedef enum maze__TileType {
  TILE_NONE,
  TILE_FLOOR,
  TILE_WALL,
  TILE_TELEPORT,
  TILE_TRAP,
  TILE_COIN,
  TILE_SWORD,
  TILE_CHEST,
  TILE_DOOR,
  TILE_KEY
} maze__TileType;

typedef struct maze__Tile {
  maze__TileType type;
  int linkedTeleportTile;
  int linkedDoorTile;
  bool isCoinCollected;
  bool isSwordCollected;
  bool isChestCollected;
  bool isKeyCollected;
  bool isDoorOpen;
  engine_Sprite *sprite;
  engine_Anim *anim;
  game_AABB aabb;
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
  float chestDespawnTimer;
  int chestScore;
  float chestScoreTimer;
  bool reverseAfterTeleport;
  int keyID;
  bool hasKeySpawned;
  engine_Texture *tileset;
  maze__Tile *tiles;
} maze__Maze;

// --- Global state ---

extern maze__Maze g_maze;
