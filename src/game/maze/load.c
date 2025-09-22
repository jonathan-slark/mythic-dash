#include <assert.h>
#include <cute_headers/cute_tiled.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../internal.h"
#include "../maze/maze.h"
#include "internal.h"
#include "log/log.h"

// --- Types ---

// Temporary storage for tiles from map tileset during loading
typedef struct {
  maze_TileType type;
  int           teleportType;
  int           trapType;
  int           doorType;
  int           keyType;
  int           animCount;
} MapTile;

// --- Constants ---

constexpr size_t   BUFFER_SIZE         = 1024;
static const float ANIM_FRAME_TIME     = 0.1f;
static const int   TILE_PROPERTY_COUNT = 8;
static const int   TELEPORT_TYPES      = 3;
static const int   MAP_PROPERTY_COUNT  = 1;

// --- Helper functions ---

static bool getTileProperties(cute_tiled_map_t* map, MapTile** tileData, int* tileCount) {
  assert(map != nullptr);
  assert(tileData != nullptr && *tileData == nullptr);
  assert(tileCount != nullptr && *tileCount == 0);

  cute_tiled_tileset_t* tileset = map->tilesets;
  *tileCount                    = tileset->tilecount;
  if (*tileCount <= 0) {
    LOG_FATAL(game_log, "No tiles in tileset");
    return false;
  }

  *tileData = (MapTile*) malloc(*tileCount * sizeof(MapTile));
  if (*tileData == nullptr) {
    LOG_FATAL(game_log, "Unable to allocate memory for tileset");
    return false;
  }

  int                           i    = *tileCount - 1;
  cute_tiled_tile_descriptor_t* tile = tileset->tiles;
  while (tile != nullptr) {
    if (i < 0) {
      LOG_FATAL(game_log, "Incomplete tile linked list");
      return false;
    }
    if (tile->property_count != TILE_PROPERTY_COUNT || tile->properties == nullptr) {
      LOG_FATAL(game_log, "Invalid property count in tile linked list");
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
            LOG_TRACE(game_log, "Tile %d is a teleport, type %d", i, (*tileData)[i].teleportType);
          }
        } else if (strcmp(tile->properties[j].name.ptr, "trapType") == 0) {
          (*tileData)[i].trapType = tile->properties[j].data.integer;
          if (tile->properties[j].data.integer > 0) {
            (*tileData)[i].type = TILE_TRAP;
            LOG_TRACE(game_log, "Tile %d is a trap, type %d", i, (*tileData)[i].trapType);
          }
        } else if (strcmp(tile->properties[j].name.ptr, "doorType") == 0) {
          (*tileData)[i].doorType = tile->properties[j].data.integer;
          if (tile->properties[j].data.integer > 0) {
            (*tileData)[i].type = TILE_DOOR;
            LOG_TRACE(game_log, "Tile %d is a door, type %d", i, (*tileData)[i].doorType);
          }
        } else if (strcmp(tile->properties[j].name.ptr, "keyType") == 0) {
          (*tileData)[i].keyType = tile->properties[j].data.integer;
          if (tile->properties[j].data.integer > 0) {
            (*tileData)[i].type = TILE_KEY;
            LOG_TRACE(game_log, "Tile %d is a key, type %d", i, (*tileData)[i].keyType);
          }
        }
      } else if (tile->properties[j].type == CUTE_TILED_PROPERTY_BOOL) {
        if (strcmp(tile->properties[j].name.ptr, "isCoin") == 0 && tile->properties[j].data.boolean) {
          (*tileData)[i].type = TILE_COIN;
          LOG_TRACE(game_log, "Tile %d isCoin", i);
        } else if (strcmp(tile->properties[j].name.ptr, "isWall") == 0 && tile->properties[j].data.boolean) {
          (*tileData)[i].type = TILE_WALL;
          LOG_TRACE(game_log, "Tile %d isWall", i);
        } else if (strcmp(tile->properties[j].name.ptr, "isSword") == 0 && tile->properties[j].data.boolean) {
          LOG_TRACE(game_log, "Tile %d isSword", i);
          (*tileData)[i].type = TILE_SWORD;
        } else if (strcmp(tile->properties[j].name.ptr, "isChest") == 0 && tile->properties[j].data.boolean) {
          LOG_TRACE(game_log, "Tile %d isChest", i);
          (*tileData)[i].type = TILE_CHEST;
        }
      }
    }
    tile = tile->next;
    i--;
  }

  return true;
}

static bool getMapProperties(cute_tiled_map_t* map, int level) {
  if (map->property_count != MAP_PROPERTY_COUNT || map->properties == nullptr) {
    LOG_FATAL(game_log, "Invalid map property count");
    return false;
  }

  for (int i = 0; i < map->property_count; i++) {
    if (map->properties[i].type == CUTE_TILED_PROPERTY_BOOL) {
      if (strcmp(map->properties[i].name.ptr, "reverseAfterTeleport") == 0) {
        g_maze[level].reverseAfterTeleport = map->properties[i].data.boolean;
        LOG_INFO(game_log, "reverseAfterTeleport: %s", g_maze[level].reverseAfterTeleport ? "true" : "false");
      }
    }
  }

  return true;
}

static bool createMaze(cute_tiled_map_t* map, MapTile tileData[], int level) {
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
  int                   teleportIDs[TELEPORT_TYPES][2];
  int                   keyCount[MAX_KEY_TYPES];
  int                   keyIDs[MAX_KEY_TYPES];
  int                   doorCount[MAX_KEY_TYPES];
  int                   doorIDs[MAX_KEY_TYPES];

  if (layer == nullptr || tileset == nullptr || count <= 0 || rows <= 0 || cols <= 0 || tileWidth <= 0 ||
      tileHeight <= 0 || tileCols <= 0) {
    LOG_FATAL(game_log, "Invalid map file");
    return false;
  }

  // Get number of layers
  int layerCount = 0;
  while (layer != nullptr) {
    layerCount++;
    layer = layer->next;
  }

  maze_Tile* tiles = (maze_Tile*) malloc(count * sizeof(maze_Tile) * layerCount);
  if (tiles == nullptr) {
    LOG_FATAL(game_log, "Unable to allocate memory for maze tiles");
    return false;
  }

  for (int i = 0; i < TELEPORT_TYPES; i++) {
    teleportCount[i] = 0;
  }

  for (int i = 0; i < MAX_KEY_TYPES; i++) {
    keyCount[i]  = 0;
    doorCount[i] = 0;
  }

  layer        = map->layers;
  int layerNum = 0;
  while (layer != nullptr) {
    for (int i = 0; i < count; i++) {
      maze_TileType  type   = TILE_NONE;
      engine_Sprite* sprite = nullptr;
      engine_Anim*   anim   = nullptr;
      game_AABB      aabb   = {};

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
        aabb   = (game_AABB) { .min = min, .max = max };

        if (animCount > 0) {
          LOG_TRACE(game_log, "New animation: layer: %d, tile %d, %d, frame count: %d", layerNum, row, col, animCount);
          anim = engine_createAnim(
              sprite,
              tilesetRow,
              tilesetCol,
              animCount,
              ANIM_FRAME_TIME,
              inset,
              type != TILE_TRAP ||
                  (type == TILE_TRAP && tileData[tileId].trapType != TRAP_SPIKE &&
                   tileData[tileId].trapType != TRAP_DOOR)
          );
        }

        if (tileData[tileId].type == TILE_KEY) {
          int keyType = tileData[tileId].keyType;
          if (keyType > 0) {
            if (keyType > MAX_KEY_TYPES) {
              LOG_WARN(game_log, "Invalid key type: %d", keyType);
            } else {
              keyType -= 1;
              if (keyCount[keyType] != -1) {
                if (keyCount[keyType] == 0) {
                  keyIDs[keyType] = tileIdx;
                  keyCount[keyType]++;
                } else {
                  LOG_WARN(game_log, "More than one key of the same type found in map");
                  keyCount[keyType] = -1;
                }
              }
            }
          }
        } else if (tileData[tileId].type == TILE_DOOR) {
          int doorType = tileData[tileId].doorType;
          if (doorType > 0) {
            if (doorType > MAX_KEY_TYPES) {
              LOG_WARN(game_log, "Invalid door type: %d", doorType);
            } else {
              doorType -= 1;
              if (doorCount[doorType] != -1) {
                if (doorCount[doorType] == 0) {
                  doorIDs[doorType] = tileIdx;
                  doorCount[doorType]++;
                } else {
                  LOG_WARN(game_log, "More than one door of the same type found in map");
                  doorCount[doorType] = -1;
                }
              }
            }
          }
        } else if (tileData[tileId].type == TILE_TRAP) {
          tiles[tileIdx].trapType = tileData[tileId].trapType;
        } else {
          int teleportType = tileData[tileId].teleportType;
          if (teleportType > 0) {
            if (teleportType > TELEPORT_TYPES) {
              LOG_WARN(game_log, "Invalid teleport type: %d", teleportType);
            } else {
              teleportType -= 1;
              if (teleportCount[teleportType] != -1) {
                if (teleportCount[teleportType] < 2) {
                  teleportIDs[teleportType][teleportCount[teleportType]++] = i;  // TODO: should be = tileIdx
                } else {
                  LOG_WARN(game_log, "Too many teleports of same type found in map");
                  teleportCount[teleportType] = -1;
                }
              }
            }
          }
        }
      }

      tiles[tileIdx].type               = type;
      tiles[tileIdx].linkedTeleportTile = -1;
      tiles[tileIdx].linkedDoorTile     = -1;
      tiles[tileIdx].sprite             = sprite;
      tiles[tileIdx].anim               = anim;
      tiles[tileIdx].aabb               = aabb;
    }

    layerNum++;
    layer = layer->next;
  }

  for (int i = 0; i < TELEPORT_TYPES; i++) {
    if (teleportCount[i] == 2) {
      tiles[teleportIDs[i][0]].linkedTeleportTile = teleportIDs[i][1];
      tiles[teleportIDs[i][1]].linkedTeleportTile = teleportIDs[i][0];
      LOG_INFO(game_log, "Linked teleports: %d and %d", teleportIDs[i][0], teleportIDs[i][1]);
    } else if (teleportCount[i] == 1) {
      LOG_WARN(game_log, "Found one teleport but not matching twin");
    }
  }

  for (int i = 0; i < MAX_KEY_TYPES; i++) {
    if (keyCount[i] == 1) {
      tiles[keyIDs[i]].linkedDoorTile = doorIDs[i];
      LOG_INFO(game_log, "Linked key and door: %d and %d", keyIDs[i], doorIDs[i]);
    }
  }

  LOG_INFO(
      game_log,
      "Created maze: %d x %d tiles (%d total; size: %d x %d), %d layer%s",
      cols,
      rows,
      count,
      tileWidth,
      tileHeight,
      layerCount,
      layerCount == 1 ? "" : "s"
  );

  g_maze[level] = (maze_Maze) {
    .rows       = rows,
    .cols       = cols,
    .count      = count,
    .tileWidth  = tileWidth,
    .tileHeight = tileHeight,
    .layerCount = layerCount,
    .tileset    = nullptr,
    .tiles      = tiles,
  };

  g_maze[level].keyCount = 0;
  for (int i = 0; i < MAX_KEY_TYPES; i++) {
    g_maze[level].keyCount += keyCount[i];
    if (keyCount[i] == 1) {
      g_maze[level].keyIDs[i] = keyIDs[i];
    } else {
      g_maze[level].keyIDs[i] = -1;
    }
  }

  GAME_TRY(getMapProperties(map, level));

  return true;
}

// Convert cute tiled map to our format
static bool convertMap(cute_tiled_map_t* map, int level) {
  assert(map != nullptr);

  MapTile* tileData  = nullptr;
  int      tileCount = 0;
  GAME_TRY(getTileProperties(map, &tileData, &tileCount));
  assert(tileData != nullptr);
  assert(tileCount > 0);
  if (!createMaze(map, tileData, level)) {
    free(tileData);
    return false;
  }
  assert(tileData != nullptr);
  free(tileData);
  return true;
}

static void destroyMaze(int level) {
  if (g_maze[level].tiles != nullptr) {
    for (int i = 0; i < g_maze[level].count; i++) {
      if (g_maze[level].tiles[i].sprite != nullptr) engine_destroySprite(&g_maze[level].tiles[i].sprite);
    }

    free(g_maze[level].tiles);
    g_maze[level].tiles = nullptr;
  }
}

static bool loadMazetileset(cute_tiled_map_t* map, int level) {
  assert(map != nullptr);
  if (map->tilesets == nullptr || map->tilesets->image.ptr == nullptr) {
    LOG_FATAL(game_log, "There is no tileset in the map file");
    return false;
  }

  char buffer[BUFFER_SIZE] = ASSET_DIR "map/";

  size_t filenameLen = strlen(map->tilesets->image.ptr);
  size_t folderLen   = strlen(buffer);
  if (folderLen + filenameLen >= sizeof buffer - 1) {
    LOG_FATAL(game_log, "Tileset file name too long to fit buffer");
    return false;
  }

  strncat(buffer, map->tilesets->image.ptr, filenameLen);
  GAME_TRY(g_maze[level].tileset = engine_textureLoad(buffer));

  return true;
}

static void unloadMazetileset(int level) {
  assert(g_maze[level].tileset != nullptr);
  engine_textureUnload(&g_maze[level].tileset);
}

void countCoins(int level) {
  for (int layerNum = 0; layerNum < g_maze[level].layerCount; layerNum++) {
    for (int i = 0; i < g_maze[level].count; i++) {
      int idx = i + layerNum * g_maze[level].count;
      if (g_maze[level].tiles[idx].type == TILE_COIN || g_maze[level].tiles[idx].type == TILE_SWORD) {
        g_maze[level].coinCount++;
      }
    }
  }
  LOG_INFO(game_log, "Coin count: %d", g_maze[level].coinCount);
}

void findChest(int level) {
  int count = 0;
  for (int layerNum = 0; layerNum < g_maze[level].layerCount; layerNum++) {
    for (int i = 0; i < g_maze[level].count; i++) {
      int idx = i + layerNum * g_maze[level].count;
      if (g_maze[level].tiles[idx].type == TILE_CHEST) {
        count++;
        g_maze[level].chestID = idx;
      }
    }
  }

  if (count == 0) {
    LOG_WARN(game_log, "No chest found in the map");
    g_maze[level].chestID = -1;
  } else if (count == 1) {
    LOG_INFO(game_log, "Found chest: %d", g_maze[level].chestID);
  } else {
    LOG_WARN(game_log, "More than one chest found in the map");
  }
}

// --- Maze functions ---

bool maze_init(void) {
  bool success = false;
  int  level   = -1;
  // File names start at 1 but array starts at 0
  for (int fileNum = 1; fileNum <= LEVEL_COUNT; fileNum++) {
    level = fileNum - 1;
    cute_tiled_map_t* map;

    LOG_INFO(game_log, "--- Map %d ---", fileNum);

    char format[] = ASSET_DIR "map/maze%02d.tmj";
    int  size     = snprintf(nullptr, 0, format, fileNum);
    char file[size + 1];
    snprintf(file, sizeof file, format, fileNum);

    GAME_TRY(map = cute_tiled_load_map_from_file(file, nullptr));
    LOG_INFO(game_log, "Map loaded: %s (%d x %d)", file, map->width, map->height);

    success = convertMap(map, level);
    if (!success) {
      LOG_FATAL(game_log, "Failed to convert map");
      cute_tiled_free_map(map);
      break;
    }

    success = loadMazetileset(map, level);
    if (!success) {
      LOG_FATAL(game_log, "Failed to load map tileset");
      cute_tiled_free_map(map);
      break;
    }

    cute_tiled_free_map(map);
    countCoins(level);
    findChest(level);
    maze_reset(level);
  }

  if (!success) {
    for (int i = 0; i <= level; i++) {
      destroyMaze(i);
    }
    return false;
  }

  LOG_INFO(game_log, "--- Maps loaded ---");

  return true;
}

void maze_shutdown(void) {
  for (int i = 0; i < LEVEL_COUNT; i++) {
    unloadMazetileset(i);
    destroyMaze(i);
  }
}
