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

// Temporary storage for tiles from map tileset during loading
typedef struct MapTile {
  TileType type;
  int      animCount;
} MapTile;

typedef struct MazeTile {
  TileType       type;
  int            linkedTeleportTile;
  engine_Sprite* sprite;
  engine_Anim*   anim;
  game__AABB     aabb;
} MazeTile;

typedef struct Maze {
  int             rows;
  int             cols;
  int             count;
  int             tileWidth;
  int             tileHeight;
  int             layerCount;
  engine_Texture* tileset;
  MazeTile*       tiles;
} Maze;

// --- Constants ---

#define OVERLAY_COLOUR_MAZE_WALL (Color){ 0, 0, 128, 128 }
const Vector2      MAZE_ORIGIN = { 8.0f, 16.0f };  // Screen offset to the actual maze
static const char* FILE_MAZE   = ASSET_DIR "map/maze01.tmj";
constexpr size_t   BUFFER_SIZE = 1024;
static const float FRAME_TIME  = (1.0f / 12.0f);

// --- Global state ---

Maze g_maze;

// --- Helper functions ---

static bool getTileProperties(cute_tiled_map_t* map, MapTile** tileData, int* tileCount) {
  assert(map != nullptr);
  assert(tileData != nullptr && *tileData == nullptr);
  assert(tileCount != nullptr && *tileCount == 0);

  cute_tiled_tileset_t* tileset = map->tilesets;
  *tileCount                    = tileset->tilecount;
  if (*tileCount <= 0) {
    LOG_FATAL(game__log, "No tiles in tileset");
    return false;
  }

  *tileData = (MapTile*) malloc(*tileCount * sizeof(MapTile));
  if (*tileData == nullptr) {
    LOG_FATAL(game__log, "Unable to allocate memory for tileset tileIsWalls");
    return false;
  }

  int                           i    = *tileCount - 1;
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
    (*tileData)[i].type      = tile->properties[0].data.boolean ? TILE_WALL : TILE_FLOOR;
    (*tileData)[i].animCount = tile->frame_count;
    tile                     = tile->next;
    i--;
  }

  return true;
}

static bool createMaze(cute_tiled_map_t* map, MapTile tileData[], int tileCount) {
  assert(map != nullptr && map->layers != nullptr);
  assert(tileCount > 0);

  cute_tiled_layer_t*   layer      = map->layers;
  cute_tiled_tileset_t* tileset    = map->tilesets;
  int                   count      = layer->data_count;
  int                   rows       = map->height;
  int                   cols       = map->width;
  int                   tileWidth  = tileset->tilewidth;
  int                   tileHeight = tileset->tileheight;
  int                   tileCols   = tileset->columns;

  if (layer == nullptr || tileset == nullptr || count <= 0 || rows <= 0 || cols <= 0 || tileWidth <= 0 ||
      tileHeight <= 0 || tileCols <= 0) {
    LOG_FATAL(game__log, "Invalid map file");
    return false;
  }

  // Get number of layers
  int layerCount = 0;
  while (layer != nullptr) {
    layerCount++;
    layer = layer->next;
  }

  MazeTile* tiles = (MazeTile*) malloc(count * sizeof(MazeTile) * layerCount);
  if (tiles == nullptr) {
    LOG_FATAL(game__log, "Unable to allocate memory for maze tiles");
    return false;
  }

  layer        = map->layers;
  int layerNum = 0;
  while (layer != nullptr) {
    for (int i = 0; i < count; i++) {
      TileType       type               = TILE_NONE;
      int            linkedTeleportTile = -1;  // Currently unused
      engine_Sprite* sprite             = nullptr;
      engine_Anim*   anim               = nullptr;
      game__AABB     aabb               = {};

      int     row   = i / cols;
      int     col   = i % cols;
      Vector2 pos   = { (float) col * tileWidth, (float) row * tileHeight };
      Vector2 min   = pos;
      pos           = Vector2Add(pos, MAZE_ORIGIN);
      Vector2 size  = { (float) tileWidth, (float) tileHeight };
      Vector2 inset = { 0.0f, 0.0f };
      Vector2 max   = { (float) (col + 1) * tileWidth, (float) (row + 1) * tileHeight };

      int tileIdx = i + layerNum * count;
      if (layer->data[i]) {
        int tileId     = layer->data[i] - 1;
        int tilesetRow = tileId / tileCols;
        int tilesetCol = tileId % tileCols;
        int animCount  = tileData[tileId].animCount;

        type   = tileData[tileId].type;
        sprite = engine_createSpriteFromSheet(pos, size, tilesetRow, tilesetCol, inset);
        aabb   = (game__AABB) { .min = min, .max = max };

        if (animCount > 0) {
          LOG_INFO(game__log, "New animation: layer: %d, tile %d, %d, frame count: %d", layerNum, row, col, animCount);
          anim = engine_createAnim(sprite, tilesetRow, tilesetCol, animCount, FRAME_TIME, inset);
        }
      }

      tiles[tileIdx].type               = type;
      tiles[tileIdx].linkedTeleportTile = linkedTeleportTile;
      tiles[tileIdx].sprite             = sprite;
      tiles[tileIdx].anim               = anim;
      tiles[tileIdx].aabb               = aabb;
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
  MapTile* tileData  = nullptr;
  int      tileCount = 0;
  GAME_TRY(getTileProperties(map, &tileData, &tileCount));
  assert(tileData != nullptr);
  assert(tileCount > 0);
  if (!createMaze(map, tileData, tileCount)) {
    free(tileData);
    return false;
  }
  assert(tileData != nullptr);
  free(tileData);
  return true;
}

static void destroyMaze(void) {
  if (g_maze.tiles != nullptr) {
    for (int i = 0; i < g_maze.count; i++) {
      if (g_maze.tiles[i].sprite != nullptr) engine_destroySprite(&g_maze.tiles[i].sprite);
    }

    free(g_maze.tiles);
    g_maze.tiles = nullptr;
  }
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

  int result = strncat_s(buffer, sizeof buffer - strlen(buffer) - 1, map->tilesets->image.ptr, filenameLen);
  if (result != 0) {
    LOG_FATAL(game__log, "strncat_s failed: %s", strerror(result));
    return false;
  }
  GAME_TRY(g_maze.tileset = engine_textureLoad(buffer));

  return true;
}

static void unloadMazetileset(void) { engine_textureUnload(&g_maze.tileset); }

static MazeTile* getTileAt(Vector2 pos) {
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
    destroyMaze();
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
