// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"

// --- Maze functions ---

[[nodiscard]] bool maze_init(void);
void               maze_shutdown(void);
game__AABB         maze_getAABB(Vector2 pos);
bool               maze_isWall(Vector2 pos);
bool               maze_isTeleport(Vector2 pos, Vector2* dest);
void               maze_tilesOverlay(void);
void               maze_draw(void);
void               maze_update(float frameTime);
game__Tile         maze_getTile(Vector2 pos);
Vector2            maze_getPos(game__Tile tile);
int                maze_manhattanDistance(game__Tile nextTile, game__Tile targetTile);
game__Tile         maze_doubleVectorBetween(game__Tile from, game__Tile to);
int                maze_getRows(void);
int                maze_getCols(void);
bool               maze_isCoin(Vector2 pos);
void               maze_pickupCoin(Vector2 pos);
void               maze_reset(void);
int                maze_getCoinCount(void);
bool               maze_isSword(Vector2 pos);
void               maze_pickupSword(Vector2 pos);
bool               maze_isChest(Vector2 pos);
void               maze_pickupChest(Vector2 pos, int score);
