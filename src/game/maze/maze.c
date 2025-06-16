#include <assert.h>
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include "../internal.h"
#include "internal.h"

// --- Constants ---

#define OVERLAY_COLOUR_MAZE_WALL (Color){ 0, 0, 128, 128 }
const Vector2 MAZE_ORIGIN = { 8.0f, 16.0f };  // Screen offset to the actual maze

// --- Global state ---

Maze g_maze;

static MazeTile* getTileAt(Vector2 pos) {
  int row = (int) (pos.y / g_maze.tileHeight);
  int col = (int) (pos.x / g_maze.tileWidth);
  assert(row >= 0 && row < g_maze.rows);
  assert(col >= 0 && col < g_maze.cols);
  return &g_maze.tiles[row * g_maze.cols + col];
}

// --- Maze functions ---

game__AABB maze_getAABB(Vector2 pos) {
  MazeTile* tile = getTileAt(pos);
  return tile->aabb;
}

bool maze_isWall(Vector2 pos) {
  MazeTile* tile = getTileAt(pos);
  return tile->type == TILE_WALL;
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
