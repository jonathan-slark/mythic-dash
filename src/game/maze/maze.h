// clang-format Language: C
#pragma once

#include <engine/engine.h>
#include <raylib.h>
#include "../internal.h"

// --- Maze functions ---

[[nodiscard]] bool maze_init(void);
void               maze_shutdown(void);
game_AABB          maze_getAABB(Vector2 pos);
bool               maze_isWall(Vector2 pos);
bool               maze_isTeleport(Vector2 pos, Vector2* dest);
void               maze_tilesOverlay(void);
void               maze_draw(void);
void               maze_update(float frameTime);
game_Tile          maze_getTile(Vector2 pos);
Vector2            maze_getPos(game_Tile tile);
int                maze_manhattanDistance(game_Tile nextTile, game_Tile targetTile);
game_Tile          maze_doubleVectorBetween(game_Tile from, game_Tile to);
int                maze_getRows(void);
int                maze_getCols(void);
bool               maze_isCoin(Vector2 pos);
void               maze_pickupCoin(Vector2 pos);
void               maze_reset(void);
int                maze_getCoinCount(void);
bool               maze_isSword(Vector2 pos);
void               maze_pickupSword(Vector2 pos);
bool               maze_isChest(Vector2 pos);
bool               maze_isTrap(Vector2 pos);
void               maze_pickupChest(Vector2 pos, int score);
engine_Texture*    maze_getTileSet(void);
bool               maze_reverseAfterTeleport(void);
