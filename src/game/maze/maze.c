#include "maze.h"
#include <assert.h>
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include <stdlib.h>
#include "../asset/asset.h"
#include "../internal.h"
#include "../player/player.h"
#include "internal.h"

// --- Constants ---

#define OVERLAY_COLOUR_MAZE_WALL (Color){ 0, 0, 128, 128 }
const Vector2 MAZE_ORIGIN         = { 8.0f, 8.0f };  // Screen offset to the actual maze
const float   CHEST_DESPAWN_TIMER = 10.0f;
const float   CHEST_SCORE_TIMER   = 2.0f;
const Vector2 CHEST_SCORE_OFFSET  = { 1.0f, 4.0f };

// --- Global state ---

maze_Maze g_maze[];

static maze_Tile* getTileAt(Vector2 pos, int layer, int level) {
  int row = (int) (pos.y / g_maze[level].tileHeight);
  int col = (int) (pos.x / g_maze[level].tileWidth);
  assert(row >= 0 && row < g_maze[level].rows);
  assert(col >= 0 && col < g_maze[level].cols);
  return &g_maze[level].tiles[row * g_maze[level].cols + col + layer * g_maze[level].count];
}

// --- Helper functions ---

// Chest appears at regular intervals of player collecting coins
static void checkChestSpawn(int level) {
  if (g_maze[level].chestID == -1) return;

  for (int i = 0; i < CHEST_SPAWN_COUNT; i++) {
    if (!g_maze[level].hasChestSpawned[i]) {
      if (player_getCoinsCollected() >= ((i + 1) * g_maze[level].coinCount) / (CHEST_SPAWN_COUNT + 1)) {
        g_maze[level].hasChestSpawned[i]                            = true;
        g_maze[level].tiles[g_maze[level].chestID].isChestCollected = false;
        g_maze[level].chestDespawnTimer                             = CHEST_DESPAWN_TIMER;
        LOG_INFO(game_log, "Chest spawned at %d coins", player_getCoinsCollected());
      }
    }
  }
}

// Keys appear at regular intervals of player collecting coins
static void checkKeySpawn(int level) {
  for (int i = 0; i < MAX_KEY_TYPES; i++) {
    if (g_maze[level].keyIDs[i] != -1 && !g_maze[level].hasKeySpawned[i]) {
      if (player_getCoinsCollected() >= ((i + 1) * g_maze[level].coinCount) / (MAX_KEY_TYPES + 1)) {
        g_maze[level].hasKeySpawned[i]                              = true;
        g_maze[level].tiles[g_maze[level].keyIDs[i]].isKeyCollected = false;
        LOG_INFO(game_log, "Key spawned at %d coins", player_getCoinsCollected());
      }
    }
  }
}

static void updateChestDespawnTimer(float frameTime, int level) {
  assert(frameTime >= 0.0f);
  if (g_maze[level].chestDespawnTimer == 0.0f) return;

  if (g_maze[level].tiles[g_maze[level].chestID].isChestCollected) {
    g_maze[level].chestDespawnTimer = 0.0f;
    return;
  }

  g_maze[level].chestDespawnTimer = fmaxf(g_maze[level].chestDespawnTimer - frameTime, 0.0f);
  if (g_maze[level].chestDespawnTimer == 0.0f) {
    g_maze[level].tiles[g_maze[level].chestID].isChestCollected = true;
  }
}

static void updateChestScoreTimer(float frameTime, int level) {
  assert(frameTime >= 0.0f);
  if (g_maze[level].chestScoreTimer == 0.0f) return;

  g_maze[level].chestScoreTimer = fmaxf(g_maze[level].chestScoreTimer - frameTime, 0.0f);
}

static Vector2 getChestPos(int level) {
  int idx = g_maze[level].chestID;
  int row = idx % g_maze[level].count / g_maze[level].cols;
  int col = idx % g_maze[level].count % g_maze[level].cols;
  assert(idx >= 0 && idx < g_maze[level].count * g_maze[level].layerCount);
  assert(row >= 0 && row < g_maze[level].rows);
  assert(col >= 0 && col < g_maze[level].cols);
  return (Vector2) { col * g_maze[level].tileWidth, row * g_maze[level].tileHeight };
}

// --- Maze functions ---

game_AABB maze_getAABB(Vector2 pos) {
  maze_Tile* tile = getTileAt(pos, 0, game_getLevel());
  return tile->aabb;
}

bool maze_isWall(Vector2 pos, bool isPlayer) {
  maze_Tile* tile0 = getTileAt(pos, 0, game_getLevel());
  maze_Tile* tile1 = getTileAt(pos, 1, game_getLevel());
  // Open door only lets player through
  return tile0->type == TILE_WALL || (isPlayer && tile1->type == TILE_DOOR && !tile1->isDoorOpen) ||
         (!isPlayer && tile1->type == TILE_DOOR);
}

bool maze_isCoin(Vector2 pos) {
  maze_Tile* tile = getTileAt(pos, 1, game_getLevel());
  return tile->type == TILE_COIN && !tile->isCoinCollected;
}

bool maze_isChest(Vector2 pos) {
  maze_Tile* tile = getTileAt(pos, 1, game_getLevel());
  return tile->type == TILE_CHEST && !tile->isChestCollected;
}

bool maze_isTrap(Vector2 pos) {
  maze_Tile* tile0 = getTileAt(pos, 0, game_getLevel());
  maze_Tile* tile1 = getTileAt(pos, 1, game_getLevel());
  return tile0->type == TILE_TRAP || tile1->type == TILE_TRAP;
}

bool maze_isTrapDoor(Vector2 pos) {
  maze_Tile* tile0 = getTileAt(pos, 0, game_getLevel());
  maze_Tile* tile1 = getTileAt(pos, 1, game_getLevel());
  return (tile0->type == TILE_TRAP && tile0->trapType == TRAP_DOOR) ||
         (tile1->type == TILE_TRAP && tile1->trapType == TRAP_DOOR);
}

bool maze_isKey(Vector2 pos) {
  maze_Tile* tile = getTileAt(pos, 1, game_getLevel());
  return tile->type == TILE_KEY && !tile->isKeyCollected;
}

void maze_pickupCoin(Vector2 pos) {
  maze_Tile* tile       = getTileAt(pos, 1, game_getLevel());
  tile->isCoinCollected = true;
}

int maze_getCoinCount(void) { return g_maze[game_getLevel()].coinCount; }

bool maze_isSword(Vector2 pos) {
  maze_Tile* tile = getTileAt(pos, 1, game_getLevel());
  return tile->type == TILE_SWORD && !tile->isSwordCollected;
}

void maze_pickupSword(Vector2 pos) {
  maze_Tile* tile        = getTileAt(pos, 1, game_getLevel());
  tile->isSwordCollected = true;
}

void maze_pickupChest(Vector2 pos, int score) {
  int        level              = game_getLevel();
  maze_Tile* tile               = getTileAt(pos, 1, level);
  tile->isChestCollected        = true;
  g_maze[level].chestScore      = score;
  g_maze[level].chestScoreTimer = CHEST_SCORE_TIMER;
}

void maze_pickupKey(Vector2 pos) {
  int        level     = game_getLevel();
  maze_Tile* tile      = getTileAt(pos, 1, level);
  tile->isKeyCollected = true;
  LOG_INFO(game_log, "Key collected, door: %d", tile->linkedDoorTile);
  assert(tile->linkedDoorTile >= 0);
  g_maze[level].tiles[tile->linkedDoorTile].isDoorOpen = true;
}

void maze_trapTriggered(Vector2 pos) {
  maze_Tile* tile0 = getTileAt(pos, 0, game_getLevel());
  maze_Tile* tile1 = getTileAt(pos, 1, game_getLevel());
  if (tile0->type == TILE_TRAP) {
    tile0->hasTrapTriggered = true;
  } else if (tile1->type == TILE_TRAP) {
    tile1->hasTrapTriggered = true;
  }
}

bool maze_isTeleport(Vector2 pos, Vector2* dest) {
  int level = game_getLevel();
  // TODO: why is this layer 0?
  maze_Tile* tile   = getTileAt(pos, 0, level);
  int        linked = tile->linkedTeleportTile;
  if (linked >= 0) {
    *dest = g_maze[level].tiles[linked].aabb.min;
    return true;
  }
  return false;
}

void maze_tilesOverlay(void) {
  int level = game_getLevel();
  for (int i = 0; i < g_maze[level].count; i++) {
    if (maze_isWall(g_maze[level].tiles[i].aabb.min, false)) {
      game_drawAABBOverlay(g_maze[level].tiles[i].aabb, OVERLAY_COLOUR_MAZE_WALL);
    }
  }
}

void maze_draw(void) {
  int level = game_getLevel();
  assert(g_maze[level].layerCount >= 0);
  assert(g_maze[level].count > 0);
  assert(g_maze[level].tileset != nullptr);

  for (int layerNum = 0; layerNum < g_maze[level].layerCount; layerNum++) {
    for (int i = 0; i < g_maze[level].count; i++) {
      int idx = i + layerNum * g_maze[level].count;
      if (g_maze[level].tiles[idx].type != TILE_NONE) {
        if ((g_maze[level].tiles[idx].type == TILE_COIN && !g_maze[level].tiles[idx].isCoinCollected) ||
            (g_maze[level].tiles[idx].type == TILE_SWORD && !g_maze[level].tiles[idx].isSwordCollected) ||
            (g_maze[level].tiles[idx].type == TILE_CHEST && !g_maze[level].tiles[idx].isChestCollected) ||
            (g_maze[level].tiles[idx].type == TILE_KEY && !g_maze[level].tiles[idx].isKeyCollected) ||
            (g_maze[level].tiles[idx].type == TILE_DOOR && !g_maze[level].tiles[idx].isDoorOpen) ||
            (g_maze[level].tiles[idx].type != TILE_COIN && g_maze[level].tiles[idx].type != TILE_SWORD &&
             g_maze[level].tiles[idx].type != TILE_CHEST && g_maze[level].tiles[idx].type != TILE_KEY &&
             g_maze[level].tiles[idx].type != TILE_DOOR)) {
          engine_Sprite* sprite = g_maze[level].tiles[idx].sprite;
          assert(sprite != nullptr);
          engine_drawSprite(g_maze[level].tileset, sprite, WHITE);
        }
      }
    }
  }

  if (g_maze[level].chestScoreTimer > 0.0f) {
    Vector2 pos = Vector2Add(POS_ADJUST(getChestPos(level)), CHEST_SCORE_OFFSET);
    engine_fontPrintf(asset_getFontTiny(), pos.x, pos.y, WHITE, "%d", g_maze[level].chestScore);
  }
}

void maze_update(float frameTime) {
  int level = game_getLevel();
  assert(g_maze[level].layerCount >= 0);
  assert(g_maze[level].count > 0);

  for (int layerNum = 0; layerNum < g_maze[level].layerCount; layerNum++) {
    for (int i = 0; i < g_maze[level].count; i++) {
      int idx = i + layerNum * g_maze[level].count;
      if (g_maze[level].tiles[idx].type != TILE_NONE) {
        if (((g_maze[level].tiles[idx].type == TILE_TRAP && g_maze[level].tiles[idx].trapType == TRAP_SPIKE) ||
             (g_maze[level].tiles[idx].type == TILE_TRAP && g_maze[level].tiles[idx].trapType == TRAP_DOOR)) &&
            !g_maze[level].tiles[idx].hasTrapTriggered)
          continue;
        engine_Anim* anim = g_maze[level].tiles[idx].anim;
        if (anim != nullptr) {
          engine_updateAnim(anim, frameTime);
        }
      }
    }
  }

  checkChestSpawn(level);
  checkKeySpawn(level);
  updateChestDespawnTimer(frameTime, level);
  updateChestScoreTimer(frameTime, level);
}

game_Tile maze_getTile(Vector2 pos) { return (game_Tile) { pos.x / TILE_SIZE, pos.y / TILE_SIZE }; }

Vector2 maze_getPos(game_Tile tile) { return (Vector2) { tile.col * TILE_SIZE, tile.row * TILE_SIZE }; }

int maze_manhattanDistance(game_Tile a, game_Tile b) {
  int dx = abs(a.col - b.col);
  int dy = abs(a.row - b.row);
  return dx + dy;
}

game_Tile maze_doubleVectorBetween(game_Tile from, game_Tile to) {
  int       level  = game_getLevel();
  game_Tile diff   = { to.col - from.col, to.row - from.row };
  game_Tile target = { to.col + diff.col, to.row + diff.row };
  if (target.col < 1) target.col = 1;
  if (target.row < 1) target.row = 1;
  if (target.col >= g_maze[level].cols - 1) target.col = g_maze[level].cols - 2;
  if (target.row >= g_maze[level].rows - 1) target.row = g_maze[level].rows - 2;
  return target;
}

int maze_getRows(void) {
  int level = game_getLevel();
  assert(g_maze[level].rows > 0);
  return g_maze[level].rows;
}

int maze_getCols(void) {
  int level = game_getLevel();
  assert(g_maze[level].cols > 0);
  return g_maze[level].cols;
}

void maze_reset(int level) {
  g_maze[level].chestDespawnTimer = 0.0f;
  g_maze[level].chestScoreTimer   = 0.0f;
  for (int i = 0; i < CHEST_SPAWN_COUNT; i++) {
    g_maze[level].hasChestSpawned[i] = false;
  }
  for (int i = 0; i < MAX_KEY_TYPES; i++) {
    g_maze[level].hasKeySpawned[i] = false;
  }

  for (int layerNum = 0; layerNum < g_maze[level].layerCount; layerNum++) {
    for (int i = 0; i < g_maze[level].count; i++) {
      int idx                                   = i + layerNum * g_maze[level].count;
      g_maze[level].tiles[idx].isCoinCollected  = false;
      g_maze[level].tiles[idx].isSwordCollected = false;
      g_maze[level].tiles[idx].isChestCollected = true;  // Spawned later
      g_maze[level].tiles[idx].isKeyCollected   = true;  // Spawned later
      g_maze[level].tiles[idx].isDoorOpen       = false;
      g_maze[level].tiles[idx].hasTrapTriggered = false;
      if (g_maze[level].tiles[idx].anim != nullptr) engine_resetAnim(g_maze[level].tiles[idx].anim);
    }
  }
}

engine_Texture* maze_getTileSet(void) { return g_maze[game_getLevel()].tileset; }

bool maze_reverseAfterTeleport(void) { return g_maze[game_getLevel()].reverseAfterTeleport; }
