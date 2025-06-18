#include "maze.h"
#include <assert.h>
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include "../game.h"

// --- Constants ---

#define OVERLAY_COLOUR_MAZE_WALL (Color){ 0, 0, 128, 128 }
const Vector2 MAZE_ORIGIN = { 8.0f, 16.0f };  // Screen offset to the actual maze

// --- Global state ---

maze__Maze g_maze;

static maze__Tile* getTileAt(Vector2 pos) {
  int row = (int) (pos.y / g_maze.tileHeight);
  int col = (int) (pos.x / g_maze.tileWidth);
  assert(row >= 0 && row < g_maze.rows);
  assert(col >= 0 && col < g_maze.cols);
  return &g_maze.tiles[row * g_maze.cols + col];
}

// --- maze__Maze functions ---

game__AABB maze_getAABB(Vector2 pos) {
  maze__Tile* tile = getTileAt(pos);
  return tile->aabb;
}

bool maze_isWall(Vector2 pos) {
  maze__Tile* tile = getTileAt(pos);
  return tile->type == TILE_WALL;
}

bool maze_isTeleport(Vector2 pos, Vector2* dest) {
  maze__Tile* tile   = getTileAt(pos);
  int         linked = tile->linkedTeleportTile;
  if (linked >= 0) {
    *dest = g_maze.tiles[linked].aabb.min;
    return true;
  }
  return false;
}

void maze_tilesOverlay(void) {
  for (int i = 0; i < g_maze.count; i++) {
    if (maze_isWall(g_maze.tiles[i].aabb.min)) {
      aabb_drawOverlay(g_maze.tiles[i].aabb, OVERLAY_COLOUR_MAZE_WALL);
    }
  }
}

void maze_draw(void) {
  assert(g_maze.layerCount >= 0);
  assert(g_maze.count > 0);
  assert(g_maze.tileset != nullptr);

  for (int layerNum = 0; layerNum < g_maze.layerCount; layerNum++) {
    for (int i = 0; i < g_maze.count; i++) {
      int idx = i + layerNum * g_maze.count;
      if (g_maze.tiles[idx].type != TILE_NONE) {
        engine_Sprite* sprite = g_maze.tiles[idx].sprite;
        assert(sprite != nullptr);
        engine_drawSprite(g_maze.tileset, sprite);
      }
    }
  }
}

void maze_update(float frameTime) {
  assert(g_maze.layerCount >= 0);
  assert(g_maze.count > 0);

  for (int layerNum = 0; layerNum < g_maze.layerCount; layerNum++) {
    for (int i = 0; i < g_maze.count; i++) {
      int idx = i + layerNum * g_maze.count;
      if (g_maze.tiles[idx].type != TILE_NONE) {
        engine_Anim* anim = g_maze.tiles[idx].anim;
        if (anim != nullptr) {
          engine_updateAnim(anim, frameTime);
        }
      }
    }
  }
}

game__Tile maze_getTile(Vector2 pos) { return (game__Tile) { pos.x / TILE_SIZE, pos.y / TILE_SIZE }; }

Vector2 maze_getPos(game__Tile tile) { return (Vector2) { tile.col * TILE_SIZE, tile.row * TILE_SIZE }; }

int maze_manhattanDistance(game__Tile nextTile, game__Tile targetTile) {
  int x1    = nextTile.col;
  int y1    = nextTile.row;
  int x2    = targetTile.col;
  int y2    = targetTile.row;
  int distX = x1 < x2 ? x2 - x1 : x1 - x2;
  int distY = y1 < y2 ? y2 - y1 : y1 - y2;
  return distX + distY;
}

// Used by Inky to
game__Tile maze_doubleVectorBetween(game__Tile from, game__Tile to) {
  game__Tile diff   = { to.col - from.col, to.row - from.row };
  game__Tile target = { to.col + diff.col, to.row + diff.row };
  if (target.col < 0) target.col = 0;
  if (target.row < 0) target.row = 0;
  if (target.col >= g_maze.cols) target.col = g_maze.cols - 1;
  if (target.row >= g_maze.rows) target.row = g_maze.rows - 1;
  return target;
}

int maze_getRows(void) { return g_maze.rows; }

int maze_getCols(void) { return g_maze.cols; }
