#include <assert.h>
#include <cute_headers/cute_tiled.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include "engine/engine.h"
#include "internal.h"
#include "log/log.h"

// --- Macros ---
#define RETURN_FAIL() \
  do {                \
    unload(&maze);    \
    return false;     \
  } while (0)

// --- Constants ---

const Vector2 MAZE_ORIGIN = { 0.0f, 8.0f };  // Screen offset to the actual maze
#define OVERLAY_COLOUR_MAZE_WALL (Color){ 128, 128, 128, 128 }

static const char* FILE_MAZE   = ASSET_DIR "map/maze01.tmj";
constexpr int      BUFFER_SIZE = 1024;

// --- Global state ---

static cute_tiled_map_t* g_maze;
static game__AABB*       g_AABBs;
static engine_Texture*   g_tileSet;

// --- Helper functions ---

static bool makeMazeAABB(void) {
  // Only use top layer
  cute_tiled_layer_t* layer = g_maze->layers;
  if (layer == nullptr) {
    LOG_FATAL(game__log, "Layer is NULL");
    return false;
  } else {
    int count = layer->data_count;
    g_AABBs   = (game__AABB*) malloc(count * sizeof(game__AABB));

    for (int i = 0; i < count; i++) {
      int row    = i / g_maze->width;
      int col    = i % g_maze->width;
      g_AABBs[i] = (game__AABB) {
        .min = (Vector2) {       col * TILE_SIZE,       row * TILE_SIZE },
        .max = (Vector2) { (col + 1) * TILE_SIZE, (row + 1) * TILE_SIZE }
      };
    }
  }
  return true;
}

static bool loadMazeTileSet(void) {
  assert(g_maze != nullptr);
  if (g_maze->tilesets == nullptr || g_maze->tilesets->image.ptr == nullptr) {
    LOG_FATAL(game__log, "Tileset path is NULL");
    return false;
  }

  char buffer[BUFFER_SIZE] = ASSET_DIR "map/";

  size_t filenameLen = strnlen_s(g_maze->tilesets->image.ptr, sizeof buffer);
  if (filenameLen == sizeof buffer) {
    LOG_FATAL(game__log, "Tileset file name too long to fit buffer");
    return false;
  }
  size_t folderLen = strnlen_s(buffer, sizeof buffer);
  if (folderLen + filenameLen >= sizeof buffer - 1) {
    LOG_FATAL(game__log, "Tileset file name too long to fit buffer");
    return false;
  }

  int result = strncat_s(buffer, sizeof buffer, g_maze->tilesets->image.ptr, filenameLen);
  if (result != 0) {
    LOG_FATAL(game__log, "strncat_s failed: %s", strerror(result));
    return false;
  }
  GAME_TRY(g_tileSet = engine_textureLoad(buffer));

  return true;
}

// --- Maze functions ---

bool maze_init(void) {
  GAME_TRY(g_maze = cute_tiled_load_map_from_file(FILE_MAZE, nullptr));
  LOG_INFO(game__log, "Map loaded: %s (%d x %d)", FILE_MAZE, g_maze->width, g_maze->height);
  GAME_TRY(loadMazeTileSet());
  GAME_TRY(makeMazeAABB());
  return true;
}

void maze_shutdown(void) { cute_tiled_free_map(g_maze); }

game__AABB maze_getAABB(Vector2 pos) {
  int row = (int) (pos.y / TILE_SIZE);
  int col = (int) (pos.x / TILE_SIZE);
  assert(row >= 0 && row < g_maze->height);
  assert(col >= 0 && col < g_maze->width);
  return g_AABBs[row * g_maze->width + col];
}

bool maze_isWall(Vector2 pos) {
  int row = (int) (pos.y / TILE_SIZE);
  int col = (int) (pos.x / TILE_SIZE);
  assert(row >= 0 && row < g_maze->height);
  assert(col >= 0 && col < g_maze->width);
  return g_maze->layers->data[row * g_maze->width + col] != 5;
}

void maze_tilesOverlay(void) {
  for (int i = 0; i < g_maze->layers->data_count; i++) {
    if (maze_isWall(g_AABBs[i].min)) {
      aabb_drawOverlay(g_AABBs[i], OVERLAY_COLOUR_MAZE_WALL);
    }
  }
}
