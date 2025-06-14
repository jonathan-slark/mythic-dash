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

// --- Types ---

typedef struct game__Maze {
  cute_tiled_map_t* map;
  game__AABB*       aabbs;
  engine_Texture*   tileSet;
  engine_Sprite**   tileSprites;  // Sprite for each tile
  bool*             tileIsWalls;  // isWalls property for each tile
  bool*             isWalls;      // Map array holding wall data
} game__Maze;

// --- Constants ---

const Vector2 MAZE_ORIGIN = { 8.0f, 16.0f };  // Screen offset to the actual maze
#define OVERLAY_COLOUR_MAZE_WALL (Color){ 128, 128, 128, 128 }

static const char* FILE_MAZE   = ASSET_DIR "map/maze01.tmj";
constexpr int      BUFFER_SIZE = 1024;

// --- Global state ---

static game__Maze g_maze;

// --- Helper functions ---

static bool makeMazeAABB(void) {
  // Only use top layer
  cute_tiled_layer_t* layer = g_maze.map->layers;
  if (layer == nullptr) {
    LOG_FATAL(game__log, "Layer is NULL");
    return false;
  } else {
    int count    = layer->data_count;
    g_maze.aabbs = (game__AABB*) malloc(count * sizeof(game__AABB));

    for (int i = 0; i < count; i++) {
      int row         = i / g_maze.map->width;
      int col         = i % g_maze.map->width;
      g_maze.aabbs[i] = (game__AABB) {
        .min = (Vector2) {       col * TILE_SIZE,       row * TILE_SIZE },
        .max = (Vector2) { (col + 1) * TILE_SIZE, (row + 1) * TILE_SIZE }
      };
    }
  }
  return true;
}

static void destroyMazeAABB(void) {
  assert(g_maze.aabbs != nullptr);
  free(g_maze.aabbs);
  g_maze.aabbs = nullptr;
}

static bool loadMazeTileSet(void) {
  assert(g_maze.map != nullptr);
  if (g_maze.map->tilesets == nullptr || g_maze.map->tilesets->image.ptr == nullptr) {
    LOG_FATAL(game__log, "Tileset path is NULL");
    return false;
  }

  char buffer[BUFFER_SIZE] = ASSET_DIR "map/";

  size_t filenameLen = strnlen_s(g_maze.map->tilesets->image.ptr, sizeof buffer);
  if (filenameLen == sizeof buffer) {
    LOG_FATAL(game__log, "Tileset file name too long to fit buffer");
    return false;
  }
  size_t folderLen = strnlen_s(buffer, sizeof buffer);
  if (folderLen + filenameLen >= sizeof buffer - 1) {
    LOG_FATAL(game__log, "Tileset file name too long to fit buffer");
    return false;
  }

  int result = strncat_s(buffer, sizeof buffer, g_maze.map->tilesets->image.ptr, filenameLen);
  if (result != 0) {
    LOG_FATAL(game__log, "strncat_s failed: %s", strerror(result));
    return false;
  }
  GAME_TRY(g_maze.tileSet = engine_textureLoad(buffer));

  return true;
}

static void unloadMazeTileSet(void) { engine_textureUnload(&g_maze.tileSet); }

static bool createMazeTileSetSprites(void) {
  assert(g_maze.map != nullptr && g_maze.map->layers != nullptr);
  assert(g_maze.tileSprites == nullptr);

  cute_tiled_tileset_t* tileset = g_maze.map->tilesets;
  int                   count   = tileset->tilecount;
  g_maze.tileSprites            = (engine_Sprite**) malloc(count * sizeof(engine_Sprite*));
  if (g_maze.tileSprites == nullptr) {
    LOG_FATAL(game__log, "Failed to allocate memory for tileset sprites");
    return false;
  }

  int width   = tileset->tilewidth;
  int height  = tileset->tileheight;
  int columns = tileset->columns;
  int rows    = count / columns;
  for (int i = 0; i < count; i++) {
    int row               = i / columns;
    int col               = i % columns;
    g_maze.tileSprites[i] = engine_createSpriteFromSheet(
        (Vector2) { 0.0f, 0.0f }, (Vector2) { width, height }, row, col, (Vector2) { 0.0f, 0.0f }
    );
  }
  LOG_INFO(game__log, "Created %d tileset (%d x %d) sprites", count, rows, columns);

  return true;
}

static void destroyMazeTileSetSprites(void) {
  assert(g_maze.tileSprites != nullptr);
  assert(g_maze.map->tilesets != nullptr);

  cute_tiled_tileset_t* tileset = g_maze.map->tilesets;
  for (int i = 0; i < tileset->tilecount; i++) {
    assert(g_maze.tileSprites[i] != nullptr);
    engine_destroySprite(&g_maze.tileSprites[i]);
  }
  free(g_maze.tileSprites);
  g_maze.tileSprites = nullptr;
}

static bool getTileProperties(void) {
  cute_tiled_tileset_t* tileset = g_maze.map->tilesets;
  int                   count   = tileset->tilecount;
  g_maze.tileIsWalls            = (bool*) malloc(count * sizeof(bool));
  if (g_maze.tileIsWalls == nullptr) {
    LOG_FATAL(game__log, "Unable to allocate memory for tileset tileIsWalls");
    return false;
  }

  int                           i    = count - 1;
  cute_tiled_tile_descriptor_t* tile = tileset->tiles;
  while (tile != nullptr) {
    if (i < 0) {
      LOG_FATAL(game__log, "Incomplete tile linked list");
      return false;
    }
    if (tile->property_count != 1 || tile->properties == nullptr) {
      LOG_FATAL(game__log, "Invalid property count in tile linked list");
      return false;
    }
    g_maze.tileIsWalls[i] = tile->properties[0].data.boolean;
    LOG_INFO(game__log, "tileIsWalls[%d]: %d", i, g_maze.tileIsWalls[i]);
    tile = tile->next;
    i--;
  }

  return true;
}

static void destroyTileProperties(void) {
  assert(g_maze.tileIsWalls != nullptr);
  free(g_maze.tileIsWalls);
  g_maze.tileIsWalls = nullptr;
}

static bool makeMazeWalls(void) {
  cute_tiled_layer_t* layer = g_maze.map->layers;
  int                 count = layer->data_count;
  g_maze.isWalls            = (bool*) malloc(count * sizeof(bool));
  if (g_maze.isWalls == nullptr) {
    LOG_FATAL(game__log, "Unable to allocate memory for isWalls");
    return false;
  }

  for (int i = 0; i < count; i++) {
    g_maze.isWalls[i] = g_maze.tileIsWalls[layer->data[i] - 1];
    LOG_INFO(game__log, "isWalls[%d]: %d", i, g_maze.isWalls[i]);
  }
  return true;
}

static void destroyMazeWalls(void) {
  assert(g_maze.isWalls != nullptr);
  free(g_maze.isWalls);
  g_maze.isWalls = nullptr;
}

// --- Maze functions ---

bool maze_init(void) {
  GAME_TRY(g_maze.map = cute_tiled_load_map_from_file(FILE_MAZE, nullptr));
  LOG_INFO(game__log, "Map loaded: %s (%d x %d)", FILE_MAZE, g_maze.map->width, g_maze.map->height);
  GAME_TRY(loadMazeTileSet());
  GAME_TRY(createMazeTileSetSprites());
  GAME_TRY(makeMazeAABB());
  GAME_TRY(getTileProperties());
  GAME_TRY(makeMazeWalls());
  return true;
}

void maze_shutdown(void) {
  destroyMazeWalls();
  destroyTileProperties();
  destroyMazeAABB();
  destroyMazeTileSetSprites();
  unloadMazeTileSet();
  cute_tiled_free_map(g_maze.map);
}

game__AABB maze_getAABB(Vector2 pos) {
  int row = (int) (pos.y / TILE_SIZE);
  int col = (int) (pos.x / TILE_SIZE);
  assert(row >= 0 && row < g_maze.map->height);
  assert(col >= 0 && col < g_maze.map->width);
  return g_maze.aabbs[row * g_maze.map->width + col];
}

bool maze_isWall(Vector2 pos) {
  int row = (int) (pos.y / TILE_SIZE);
  int col = (int) (pos.x / TILE_SIZE);
  assert(row >= 0 && row < g_maze.map->height);
  assert(col >= 0 && col < g_maze.map->width);
  return g_maze.isWalls[row * g_maze.map->width + col];
}

void maze_tilesOverlay(void) {
  cute_tiled_layer_t* layer = g_maze.map->layers;
  for (int i = 0; i < layer->data_count; i++) {
    if (maze_isWall(g_maze.aabbs[i].min)) {
      aabb_drawOverlay(g_maze.aabbs[i], OVERLAY_COLOUR_MAZE_WALL);
    }
  }
}

void maze_draw(void) {
  assert(g_maze.map->layers != nullptr);
  assert(g_maze.map->tilesets != nullptr);
  assert(g_maze.tileSprites != nullptr);

  cute_tiled_layer_t*   layer      = g_maze.map->layers;
  cute_tiled_tileset_t* tileset    = g_maze.map->tilesets;
  int                   mapWidth   = g_maze.map->width;
  int                   tileWidth  = tileset->tilewidth;
  int                   tileHeight = tileset->tileheight;
  for (int i = 0; i < layer->data_count; i++) {
    int     row           = i / mapWidth;
    int     col           = i % mapWidth;
    Vector2 pos           = { (float) col * tileWidth, (float) row * tileHeight };
    pos                   = Vector2Add(pos, MAZE_ORIGIN);
    engine_Sprite* sprite = g_maze.tileSprites[layer->data[i] - 1];
    assert(sprite != nullptr);
    engine_spriteSetPos(sprite, pos);
    engine_drawSprite(g_maze.tileSet, sprite);
  }
}
