#include <assert.h>
#include <cute_headers/cute_tiled.h>
#include <stdlib.h>
#include <string.h>
#include "../internal.h"
#include "../maze/maze.h"
#include "internal.h"
#include "log/log.h"

// --- Types ---

// Temporary storage for tiles from map tileset during loading
typedef struct MapTile {
  maze__TileType type;
  int            teleportType;
  int            animCount;
} MapTile;

// --- Constants ---

static const char* FILE_MAZE      = ASSET_DIR "map/maze01.tmj";
constexpr size_t   BUFFER_SIZE    = 1024;
static const float FRAME_TIME     = 0.1f;
static const int   PROPERTY_TYPES = 5;
static const int   TELEPORT_TYPES = 3;

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
    if (tile->property_count != PROPERTY_TYPES || tile->properties == nullptr) {
      LOG_FATAL(game__log, "Invalid property count in tile linked list");
      return false;
    }
    (*tileData)[i].type      = TILE_FLOOR;
    (*tileData)[i].animCount = tile->frame_count;
    for (int j = 0; j < tile->property_count; j++) {
      if (tile->properties[j].type == CUTE_TILED_PROPERTY_INT) {
        if (strcmp(tile->properties[j].name.ptr, "teleportType") == 0) {
          (*tileData)[i].teleportType = tile->properties[j].data.integer;
          if (tile->properties[j].data.integer > 0) {
            (*tileData)[i].type = TILE_TELEPORT;
            LOG_TRACE(game__log, "Tile %d is a teleport, type %d", i, (*tileData)[i].teleportType);
          }
        }
      } else if (tile->properties[j].type == CUTE_TILED_PROPERTY_BOOL) {
        if (strcmp(tile->properties[j].name.ptr, "isCoin") == 0 && tile->properties[j].data.boolean) {
          (*tileData)[i].type = TILE_COIN;
          LOG_TRACE(game__log, "Tile %d isCoin", i);
        } else if (strcmp(tile->properties[j].name.ptr, "isWall") == 0 && tile->properties[j].data.boolean) {
          (*tileData)[i].type = TILE_WALL;
          LOG_TRACE(game__log, "Tile %d isWall", i);
        } else if (strcmp(tile->properties[j].name.ptr, "isSword") == 0 && tile->properties[j].data.boolean) {
          LOG_TRACE(game__log, "Tile %d isSword", i);
          (*tileData)[i].type = TILE_SWORD;
        } else if (strcmp(tile->properties[j].name.ptr, "isChest") == 0 && tile->properties[j].data.boolean) {
          LOG_TRACE(game__log, "Tile %d isChest", i);
          (*tileData)[i].type = TILE_CHEST;
        }
      }
    }
    tile = tile->next;
    i--;
  }

  return true;
}

static bool createMaze(cute_tiled_map_t* map, MapTile tileData[]) {
  assert(map != nullptr && map->layers != nullptr);

  cute_tiled_layer_t*   layer      = map->layers;
  cute_tiled_tileset_t* tileset    = map->tilesets;
  int                   count      = layer->data_count;
  int                   rows       = map->height;
  int                   cols       = map->width;
  int                   tileWidth  = tileset->tilewidth;
  int                   tileHeight = tileset->tileheight;
  int                   tileCols   = tileset->columns;
  int                   teleportCount[TELEPORT_TYPES];
  int                   teleports[TELEPORT_TYPES][2];

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

  maze__Tile* tiles = (maze__Tile*) malloc(count * sizeof(maze__Tile) * layerCount);
  if (tiles == nullptr) {
    LOG_FATAL(game__log, "Unable to allocate memory for maze tiles");
    return false;
  }

  for (int i = 0; i < TELEPORT_TYPES; i++) {
    teleportCount[i] = 0;
    teleports[i][0]  = 0;
    teleports[i][1]  = 0;
  }

  layer        = map->layers;
  int layerNum = 0;
  while (layer != nullptr) {
    for (int i = 0; i < count; i++) {
      maze__TileType type   = TILE_NONE;
      engine_Sprite* sprite = nullptr;
      engine_Anim*   anim   = nullptr;
      game__AABB     aabb   = {};

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
          LOG_TRACE(game__log, "New animation: layer: %d, tile %d, %d, frame count: %d", layerNum, row, col, animCount);
          anim = engine_createAnim(sprite, tilesetRow, tilesetCol, animCount, FRAME_TIME, inset, true);
        }

        int teleportType = tileData[tileId].teleportType;
        if (teleportType > 0) {
          if (teleportType > TELEPORT_TYPES) {
            LOG_WARN(game__log, "Invalid teleport type: %d", teleportType);
          } else {
            teleportType -= 1;
            if (teleportCount[teleportType] != -1) {
              if (teleportCount[teleportType] < 2) {
                teleports[teleportType][teleportCount[teleportType]++] = i;
              } else {
                LOG_WARN(game__log, "Too many teleports of same type found in map");
                teleportCount[teleportType] = -1;
              }
            }
          }
        }
      }

      tiles[tileIdx].type               = type;
      tiles[tileIdx].linkedTeleportTile = -1;
      tiles[tileIdx].sprite             = sprite;
      tiles[tileIdx].anim               = anim;
      tiles[tileIdx].aabb               = aabb;
    }

    layerNum++;
    layer = layer->next;
  }

  for (int i = 0; i < TELEPORT_TYPES; i++) {
    if (teleportCount[i] == 2) {
      tiles[teleports[i][0]].linkedTeleportTile = teleports[i][1];
      tiles[teleports[i][1]].linkedTeleportTile = teleports[i][0];
      LOG_INFO(game__log, "Linked teleports: %d and %d", teleports[i][0], teleports[i][1]);
    } else if (teleportCount[i] == 1) {
      LOG_WARN(game__log, "Found one teleport but not matching twin");
    }
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

  g_maze = (maze__Maze) {
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
  if (!createMaze(map, tileData)) {
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

void countCoins(void) {
  for (int layerNum = 0; layerNum < g_maze.layerCount; layerNum++) {
    for (int i = 0; i < g_maze.count; i++) {
      int idx = i + layerNum * g_maze.count;
      if (g_maze.tiles[idx].type == TILE_COIN || g_maze.tiles[idx].type == TILE_SWORD) {
        g_maze.coinCount++;
      }
    }
  }
  LOG_INFO(game__log, "Coin count: %d", g_maze.coinCount);
}

void findChest(void) {
  int count = 0;
  for (int layerNum = 0; layerNum < g_maze.layerCount; layerNum++) {
    for (int i = 0; i < g_maze.count; i++) {
      int idx = i + layerNum * g_maze.count;
      if (g_maze.tiles[idx].type == TILE_CHEST) {
        count++;
        g_maze.chestID = idx;
      }
    }
  }

  if (count == 0) {
    LOG_WARN(game__log, "No chest found in the map");
  } else if (count == 1) {
    LOG_INFO(game__log, "Found chest: %d", g_maze.chestID);
  } else {
    LOG_WARN(game__log, "More than one chest found in the map");
  }
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
  countCoins();
  findChest();
  maze_reset();

  return true;
}

void maze_shutdown(void) {
  unloadMazetileset();
  destroyMaze();
}
