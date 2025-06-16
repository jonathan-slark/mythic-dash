#include "../game.h"
#include <engine/engine.h>

// --- Types ---

typedef enum TileType {
  TILE_NONE,
  TILE_FLOOR,
  TILE_WALL,
  TILE_TELEPORT
} TileType;

typedef struct MazeTile {
  TileType type;
  int linkedTeleportTile;
  engine_Sprite *sprite;
  engine_Anim *anim;
  game__AABB aabb;
} MazeTile;

typedef struct Maze {
  int rows;
  int cols;
  int count;
  int tileWidth;
  int tileHeight;
  int layerCount;
  engine_Texture *tileset;
  MazeTile *tiles;
} Maze;

// --- Global state ---

extern Maze g_maze;
