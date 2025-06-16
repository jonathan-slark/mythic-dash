#include <assert.h>
#include <cute_headers/cute_tiled.h>
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

// --- Types ---

typedef enum TileType { TILE_NONE, TILE_FLOOR, TILE_WALL, TILE_TELEPORT } TileType;

typedef struct Tile {
  TileType       type;
  int            linkedTeleportTile;
  engine_Sprite* sprite;
  game__AABB     aabb;
} Tile;

typedef struct Maze {
  int             rows;
  int             cols;
  int             count;
  int             tileWidth;
  int             tileHeight;
  int             layerCount;
  engine_Texture* tileset;
  Tile*           tiles;
} Maze;

// --- Constants ---

#define OVERLAY_COLOUR_MAZE_WALL (Color){ 0, 0, 128, 128 }
const Vector2      MAZE_ORIGIN = { 8.0f, 16.0f };  // Screen offset to the actual maze
static const char* FILE_MAZE   = ASSET_DIR "map/maze01.tmj";
constexpr int      BUFFER_SIZE = 1024;

// --- Global state ---

Maze g_maze;

// --- Helper functions ---

static bool getTileProperties(cute_tiled_map_t* map, TileType** tileTypes, int* tileTypesCount) {
  assert(map != nullptr);
  assert(tileTypes != nullptr && *tileTypes == nullptr);
  assert(tileTypesCount != nullptr && *tileTypesCount == 0);

  cute_tiled_tileset_t* tileset = map->tilesets;
  *tileTypesCount               = tileset->tilecount;
  if (*tileTypesCount <= 0) {
    LOG_FATAL(game__log, "No tiles in tileset");
    return false;
  }

  *tileTypes = (TileType*) malloc(*tileTypesCount * sizeof(TileType));
  if (*tileTypes == nullptr) {
    LOG_FATAL(game__log, "Unable to allocate memory for tileset tileIsWalls");
    return false;
  }

  int                           i    = *tileTypesCount - 1;
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
    (*tileTypes)[i] = tile->properties[0].data.boolean ? TILE_WALL : TILE_FLOOR;
    tile            = tile->next;
    i--;
  }

  return true;
}

static bool createMaze(cute_tiled_map_t* map, TileType tileTypes[], int tileTypesCount) {
  assert(map != nullptr && map->layers != nullptr);
  assert(tileTypesCount > 0);

  if (map->layers == nullptr) {
    LOG_FATAL(game__log, "There are no layers in the map file");
    return false;
  }
  if (map->tilesets == nullptr) {
    LOG_FATAL(game__log, "There is no tileset in the map file");
    return false;
  }

  cute_tiled_layer_t*   layer      = map->layers;
  cute_tiled_tileset_t* tileset    = map->tilesets;
  int                   count      = layer->data_count;
  int                   rows       = map->height;
  int                   cols       = map->width;
  int                   tileWidth  = tileset->tilewidth;
  int                   tileHeight = tileset->tileheight;
  int                   tileCols   = tileset->columns;

  // Get number of layers
  int layerCount = 0;
  while (layer != nullptr) {
    layerCount++;
    layer = layer->next;
  }

  Tile* tiles = (Tile*) malloc(count * sizeof(Tile) * layerCount);
  if (tiles == nullptr) {
    LOG_FATAL(game__log, "Unable to allocate memory for maze tiles");
    return false;
  }

  layer        = map->layers;
  int layerNum = 0;
  while (layer != nullptr) {
    for (int i = 0; i < count; i++) {
      int     row   = i / cols;
      int     col   = i % cols;
      Vector2 pos   = { (float) col * tileWidth, (float) row * tileHeight };
      Vector2 min   = pos;
      pos           = Vector2Add(pos, MAZE_ORIGIN);
      Vector2 size  = { (float) tileWidth, (float) tileHeight };
      Vector2 inset = { 0.0f, 0.0f };
      Vector2 max   = { (float) (col + 1) * tileWidth, (float) (row + 1) * tileHeight };

      int tileIdx = i + layerNum * count;
      if (!layer->data[i]) {
        tiles[tileIdx].type = TILE_NONE;
      } else {
        int tileId     = layer->data[i] - 1;
        int tilesetRow = tileId / tileCols;
        int tilesetCol = tileId % tileCols;

        tiles[tileIdx].type   = tileTypes[tileId];
        tiles[tileIdx].sprite = engine_createSpriteFromSheet(pos, size, tilesetRow, tilesetCol, inset);
        tiles[tileIdx].aabb   = (game__AABB) { .min = min, .max = max };
      }
      tiles[tileIdx].linkedTeleportTile = -1;  // Currently unused
    }

    layerNum++;
    layer = layer->next;
  }

  LOG_INFO(
      game__log,
      "Created maze: %d x %d tiles (%d total; size: %d x %d), %d layer%s",
      cols,
      rows,
      count,
      tileWidth,
      tileHeight,
      layerCount,
      layerCount == 1 ? "" : "s"
  );

  g_maze = (Maze) {
    .rows       = rows,
    .cols       = cols,
    .count      = count,
    .tileWidth  = tileWidth,
    .tileHeight = tileHeight,
    .layerCount = layerCount,
    .tileset    = nullptr,
    .tiles      = tiles,
  };

  return true;
}

// Convert cute tiled map to our format
static bool convertMap(cute_tiled_map_t* map) {
  TileType* tileTypes      = nullptr;
  int       tileTypesCount = 0;
  GAME_TRY(getTileProperties(map, &tileTypes, &tileTypesCount));
  assert(tileTypes != nullptr);
  assert(tileTypesCount > 0);
  GAME_TRY(createMaze(map, tileTypes, tileTypesCount));
  assert(tileTypes != nullptr);
  free(tileTypes);
  return true;
}

static void destroyMaze(void) {
  assert(g_maze.tiles != nullptr);

  for (int i = 0; i < g_maze.count; i++) {
    assert(g_maze.tiles[i].sprite != nullptr);
    engine_destroySprite(&g_maze.tiles[i].sprite);
  }

  free(g_maze.tiles);
  g_maze.tiles = nullptr;
}

static bool loadMazetileset(cute_tiled_map_t* map) {
  assert(map != nullptr);
  if (map->tilesets == nullptr || map->tilesets->image.ptr == nullptr) {
    LOG_FATAL(game__log, "There is no tileset in the map file");
    return false;
  }

  char buffer[BUFFER_SIZE] = ASSET_DIR "map/";

  size_t filenameLen = strnlen_s(map->tilesets->image.ptr, sizeof buffer);
  if (filenameLen == sizeof buffer) {
    LOG_FATAL(game__log, "Tileset file name too long to fit buffer");
    return false;
  }
  size_t folderLen = strnlen_s(buffer, sizeof buffer);
  if (folderLen + filenameLen >= sizeof buffer - 1) {
    LOG_FATAL(game__log, "Tileset file name too long to fit buffer");
    return false;
  }

  int result = strncat_s(buffer, sizeof buffer, map->tilesets->image.ptr, filenameLen);
  if (result != 0) {
    LOG_FATAL(game__log, "strncat_s failed: %s", strerror(result));
    return false;
  }
  GAME_TRY(g_maze.tileset = engine_textureLoad(buffer));

  return true;
}

static void unloadMazetileset(void) { engine_textureUnload(&g_maze.tileset); }

static Tile* getTileAt(Vector2 pos) {
  int row = (int) (pos.y / g_maze.tileHeight);
  int col = (int) (pos.x / g_maze.tileWidth);
  assert(row >= 0 && row < g_maze.rows);
  assert(col >= 0 && col < g_maze.cols);
  return &g_maze.tiles[row * g_maze.cols + col];
}

// --- Maze functions ---

bool maze_init(void) {
  cute_tiled_map_t* map;
  GAME_TRY(map = cute_tiled_load_map_from_file(FILE_MAZE, nullptr));
  LOG_INFO(game__log, "Map loaded: %s (%d x %d)", FILE_MAZE);
  if (!convertMap(map)) {
    cute_tiled_free_map(map);
    LOG_INFO(game__log, "Failed to convert map");
    return false;
  }
  if (!loadMazetileset(map)) {
    destroyMaze();
    cute_tiled_free_map(map);
    LOG_INFO(game__log, "Failed to load map tileset");
    return false;
  }
  cute_tiled_free_map(map);

  return true;
}

void maze_shutdown(void) {
  unloadMazetileset();
  destroyMaze();
}

game__AABB maze_getAABB(Vector2 pos) {
  Tile* tile = getTileAt(pos);
  return tile->aabb;
}

bool maze_isWall(Vector2 pos) {
  Tile* tile = getTileAt(pos);
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
  assert(g_maze.tileset != nullptr);
  assert(g_maze.count > 0);

  for (int layerNum = 0; layerNum < g_maze.layerCount; layerNum++) {
    for (int i = 0; i < g_maze.count; i++) {
      int idx = i + layerNum * g_maze.count;
      if (g_maze.tiles[idx].type != TILE_NONE) {
        assert(g_maze.tiles[idx].sprite != nullptr);
        engine_drawSprite(g_maze.tileset, g_maze.tiles[idx].sprite);
      }
    }
  }
}
