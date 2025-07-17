/*
 * maze.h: Internal to maze units, don't include in other units.
 */

#pragma once
// Clang format Language: C

#include "../internal.h"
#include <engine/engine.h>

// --- Constants ---

constexpr int CHEST_SPAWN_COUNT = 2;

// --- Types ---

typedef enum { TRAP_ACID = 1, TRAP_SPIKE, TRAP_DOOR } maze_TrapType;

typedef enum maze_TileType {
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
} maze_TileType;

typedef struct maze_Tile {
  maze_TileType type;
  int linkedTeleportTile;
  int linkedDoorTile;
  bool isCoinCollected;
  bool isSwordCollected;
  bool isChestCollected;
  bool isKeyCollected;
  bool isDoorOpen;
  bool hasTrapTriggered;
  int trapType;
  engine_Sprite *sprite;
  engine_Anim *anim;
  game_AABB aabb;
} maze_Tile;

typedef struct maze_Maze {
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
  int keyCount;
  int keyIDs[MAX_KEY_TYPES];
  bool hasKeySpawned[MAX_KEY_TYPES];
  engine_Texture *tileset;
  maze_Tile *tiles;
} maze_Maze;

// --- Global state ---

extern maze_Maze g_maze[LEVEL_COUNT];
