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

maze__Maze g_maze;

static maze__Tile* getTileAt(Vector2 pos, int layer) {
  int row = (int) (pos.y / g_maze.tileHeight);
  int col = (int) (pos.x / g_maze.tileWidth);
  assert(row >= 0 && row < g_maze.rows);
  assert(col >= 0 && col < g_maze.cols);
  return &g_maze.tiles[row * g_maze.cols + col + layer * g_maze.count];
}

// --- Helper functions ---

// Chest appears at regular intervals of player collecting coins
static void checkChestSpawn(void) {
  for (int i = 0; i < CHEST_SPAWN_COUNT; i++) {
    if (!g_maze.hasChestSpawned[i]) {
      if (player_getCoinsCollected() >= ((i + 1) * g_maze.coinCount) / (CHEST_SPAWN_COUNT + 1)) {
        g_maze.hasChestSpawned[i]                     = true;
        g_maze.tiles[g_maze.chestID].isChestCollected = false;
        g_maze.chestDespawnTimer                      = CHEST_DESPAWN_TIMER;
        LOG_INFO(game_log, "Chest spawned at %d coins", player_getCoinsCollected());
      }
    }
  }
}

// Key appears once, mid way through coin collection
static void checkKeySpawn(void) {
  if (!g_maze.hasKeySpawned) {
    if (player_getCoinsCollected() >= g_maze.coinCount / 2) {
      g_maze.hasKeySpawned                      = true;
      g_maze.tiles[g_maze.keyID].isKeyCollected = false;
      LOG_INFO(game_log, "Key spawned at %d coins", player_getCoinsCollected());
    }
  }
}

static void updateChestDespawnTimer(float frameTime) {
  assert(frameTime >= 0.0f);
  if (g_maze.chestDespawnTimer == 0.0f) return;

  if (g_maze.tiles[g_maze.chestID].isChestCollected) {
    g_maze.chestDespawnTimer = 0.0f;
    return;
  }

  g_maze.chestDespawnTimer = fmaxf(g_maze.chestDespawnTimer - frameTime, 0.0f);
  if (g_maze.chestDespawnTimer == 0.0f) {
    g_maze.tiles[g_maze.chestID].isChestCollected = true;
  }
}

static void updateChestScoreTimer(float frameTime) {
  assert(frameTime >= 0.0f);
  if (g_maze.chestScoreTimer == 0.0f) return;

  g_maze.chestScoreTimer = fmaxf(g_maze.chestScoreTimer - frameTime, 0.0f);
}

static Vector2 getChestPos(void) {
  int idx = g_maze.chestID;
  int row = idx % g_maze.count / g_maze.cols;
  int col = idx % g_maze.count % g_maze.cols;
  assert(idx >= 0 && idx < g_maze.count * g_maze.layerCount);
  assert(row >= 0 && row < g_maze.rows);
  assert(col >= 0 && col < g_maze.cols);
  return (Vector2) { col * g_maze.tileWidth, row * g_maze.tileHeight };
}

// --- Maze functions ---

game_AABB maze_getAABB(Vector2 pos) {
  maze__Tile* tile = getTileAt(pos, 0);
  return tile->aabb;
}

bool maze_isWall(Vector2 pos, bool isPlayer) {
  maze__Tile* tile0 = getTileAt(pos, 0);
  maze__Tile* tile1 = getTileAt(pos, 1);
  // Open door only lets player through
  return tile0->type == TILE_WALL || (isPlayer && tile1->type == TILE_DOOR && !tile1->isDoorOpen) ||
         (!isPlayer && tile1->type == TILE_DOOR);
}

bool maze_isCoin(Vector2 pos) {
  maze__Tile* tile = getTileAt(pos, 1);
  return tile->type == TILE_COIN && !tile->isCoinCollected;
}

bool maze_isChest(Vector2 pos) {
  maze__Tile* tile = getTileAt(pos, 1);
  return tile->type == TILE_CHEST && !tile->isChestCollected;
}

bool maze_isTrap(Vector2 pos) {
  maze__Tile* tile0 = getTileAt(pos, 0);
  maze__Tile* tile1 = getTileAt(pos, 1);
  return tile0->type == TILE_TRAP || tile1->type == TILE_TRAP;
}

bool maze_isTrapDoor(Vector2 pos) {
  maze__Tile* tile0 = getTileAt(pos, 0);
  maze__Tile* tile1 = getTileAt(pos, 1);
  return (tile0->type == TILE_TRAP && tile0->trapType == TRAP_DOOR) ||
         (tile1->type == TILE_TRAP && tile1->trapType == TRAP_DOOR);
}

bool maze_isKey(Vector2 pos) {
  maze__Tile* tile = getTileAt(pos, 1);
  return tile->type == TILE_KEY && !tile->isKeyCollected;
}

void maze_pickupCoin(Vector2 pos) {
  maze__Tile* tile      = getTileAt(pos, 1);
  tile->isCoinCollected = true;
}

int maze_getCoinCount(void) { return g_maze.coinCount; }

bool maze_isSword(Vector2 pos) {
  maze__Tile* tile = getTileAt(pos, 1);
  return tile->type == TILE_SWORD && !tile->isSwordCollected;
}

void maze_pickupSword(Vector2 pos) {
  maze__Tile* tile       = getTileAt(pos, 1);
  tile->isSwordCollected = true;
}

void maze_pickupChest(Vector2 pos, int score) {
  maze__Tile* tile       = getTileAt(pos, 1);
  tile->isChestCollected = true;
  g_maze.chestScore      = score;
  g_maze.chestScoreTimer = CHEST_SCORE_TIMER;
}

void maze_pickupKey(Vector2 pos) {
  maze__Tile* tile     = getTileAt(pos, 1);
  tile->isKeyCollected = true;
  LOG_INFO(game_log, "Key collected, door: %d", tile->linkedDoorTile);
  assert(tile->linkedDoorTile >= 0);
  g_maze.tiles[tile->linkedDoorTile].isDoorOpen = true;
}

void maze_trapTriggered(Vector2 pos) {
  maze__Tile* tile0 = getTileAt(pos, 0);
  maze__Tile* tile1 = getTileAt(pos, 1);
  if (tile0->type == TILE_TRAP) {
    tile0->hasTrapTriggered = true;
  } else if (tile1->type == TILE_TRAP) {
    tile1->hasTrapTriggered = true;
  }
}

bool maze_isTeleport(Vector2 pos, Vector2* dest) {
  // TODO: why is this layer 0?
  maze__Tile* tile   = getTileAt(pos, 0);
  int         linked = tile->linkedTeleportTile;
  if (linked >= 0) {
    *dest = g_maze.tiles[linked].aabb.min;
    return true;
  }
  return false;
}

void maze_tilesOverlay(void) {
  for (int i = 0; i < g_maze.count; i++) {
    if (maze_isWall(g_maze.tiles[i].aabb.min, false)) {
      game_drawAABBOverlay(g_maze.tiles[i].aabb, OVERLAY_COLOUR_MAZE_WALL);
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
        if ((g_maze.tiles[idx].type == TILE_COIN && !g_maze.tiles[idx].isCoinCollected) ||
            (g_maze.tiles[idx].type == TILE_SWORD && !g_maze.tiles[idx].isSwordCollected) ||
            (g_maze.tiles[idx].type == TILE_CHEST && !g_maze.tiles[idx].isChestCollected) ||
            (g_maze.tiles[idx].type == TILE_KEY && !g_maze.tiles[idx].isKeyCollected) ||
            (g_maze.tiles[idx].type == TILE_DOOR && !g_maze.tiles[idx].isDoorOpen) ||
            (g_maze.tiles[idx].type != TILE_COIN && g_maze.tiles[idx].type != TILE_SWORD &&
             g_maze.tiles[idx].type != TILE_CHEST && g_maze.tiles[idx].type != TILE_KEY &&
             g_maze.tiles[idx].type != TILE_DOOR)) {
          engine_Sprite* sprite = g_maze.tiles[idx].sprite;
          assert(sprite != nullptr);
          engine_drawSprite(g_maze.tileset, sprite, WHITE);
        }
      }
    }
  }

  if (g_maze.chestScoreTimer > 0.0f) {
    Vector2 pos = Vector2Add(POS_ADJUST(getChestPos()), CHEST_SCORE_OFFSET);
    engine_fontPrintf(asset_getFontTiny(), pos.x, pos.y, WHITE, "%d", g_maze.chestScore);
  }
}

void maze_update(float frameTime) {
  assert(g_maze.layerCount >= 0);
  assert(g_maze.count > 0);

  for (int layerNum = 0; layerNum < g_maze.layerCount; layerNum++) {
    for (int i = 0; i < g_maze.count; i++) {
      int idx = i + layerNum * g_maze.count;
      if (g_maze.tiles[idx].type != TILE_NONE) {
        if (((g_maze.tiles[idx].type == TILE_TRAP && g_maze.tiles[idx].trapType == TRAP_SPIKE) ||
             (g_maze.tiles[idx].type == TILE_TRAP && g_maze.tiles[idx].trapType == TRAP_DOOR)) &&
            !g_maze.tiles[idx].hasTrapTriggered)
          continue;
        engine_Anim* anim = g_maze.tiles[idx].anim;
        if (anim != nullptr) {
          engine_updateAnim(anim, frameTime);
        }
      }
    }
  }

  checkChestSpawn();
  checkKeySpawn();
  updateChestDespawnTimer(frameTime);
  updateChestScoreTimer(frameTime);
}

game_Tile maze_getTile(Vector2 pos) { return (game_Tile) { pos.x / TILE_SIZE, pos.y / TILE_SIZE }; }

Vector2 maze_getPos(game_Tile tile) { return (Vector2) { tile.col * TILE_SIZE, tile.row * TILE_SIZE }; }

int maze_manhattanDistance(game_Tile a, game_Tile b) {
  int dx = abs(a.col - b.col);
  int dy = abs(a.row - b.row);
  return dx + dy;
}

game_Tile maze_doubleVectorBetween(game_Tile from, game_Tile to) {
  game_Tile diff   = { to.col - from.col, to.row - from.row };
  game_Tile target = { to.col + diff.col, to.row + diff.row };
  if (target.col < 1) target.col = 1;
  if (target.row < 1) target.row = 1;
  if (target.col >= g_maze.cols - 1) target.col = g_maze.cols - 2;
  if (target.row >= g_maze.rows - 1) target.row = g_maze.rows - 2;
  return target;
}

int maze_getRows(void) {
  assert(g_maze.rows > 0);
  return g_maze.rows;
}

int maze_getCols(void) {
  assert(g_maze.cols > 0);
  return g_maze.cols;
}

void maze_reset(void) {
  g_maze.chestDespawnTimer = 0.0f;
  g_maze.chestScoreTimer   = 0.0f;
  for (int i = 0; i < CHEST_SPAWN_COUNT; i++) {
    g_maze.hasChestSpawned[i] = false;
  }

  for (int layerNum = 0; layerNum < g_maze.layerCount; layerNum++) {
    for (int i = 0; i < g_maze.count; i++) {
      int idx                            = i + layerNum * g_maze.count;
      g_maze.tiles[idx].isCoinCollected  = false;
      g_maze.tiles[idx].isSwordCollected = false;
      g_maze.tiles[idx].isChestCollected = true;  // Spawned later
      g_maze.tiles[idx].isKeyCollected   = true;  // Spawned later
      g_maze.tiles[idx].isDoorOpen       = false;
      g_maze.tiles[idx].hasTrapTriggered = false;
      if (g_maze.tiles[idx].anim != nullptr) engine_resetAnim(g_maze.tiles[idx].anim);
    }
  }
}

engine_Texture* maze_getTileSet(void) { return g_maze.tileset; }

bool maze_reverseAfterTeleport(void) { return g_maze.reverseAfterTeleport; }
