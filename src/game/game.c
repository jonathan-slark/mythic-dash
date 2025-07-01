#include <assert.h>
#include <engine/engine.h>
#include <game/game.h>
#include <log/log.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include "asset/asset.h"
#include "creature/creature.h"
#include "debug/debug.h"
#include "draw/draw.h"
#include "internal.h"
#include "maze/maze.h"
#include "player/player.h"

// --- Constants ---

#ifndef NDEBUG
static const log_Config LOG_CONFIG_GAME = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "GAME"
};
#else
static const log_Config LOG_CONFIG_GAME = {
  .minLevel      = LOG_LEVEL_INFO,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "GAME"
};
#endif

#ifndef NDEBUG
static const float FPS[] = { 15, 30, 60, 0 };
#endif

const float BASE_SLOP       = 0.35f;
const float BASE_DT         = (1.0f / 144.0f);
const float MIN_SLOP        = 0.05f;
const float MAX_SLOP        = 2.5f;
const float OVERLAP_EPSILON = 2e-5f;

// --- Global state ---

log_Log*   game_log;
static int g_level = 1;
#ifndef NDEBUG
static size_t g_fpsIndex = COUNT(FPS) - 1;
#endif

// --- Helper functions ---

#ifndef NDEBUG
static void checkFPSKeys(void) {
  if (engine_isKeyPressed(KEY_MINUS)) {
    g_fpsIndex = (g_fpsIndex == 0) ? COUNT(FPS) - 1 : g_fpsIndex - 1;
  }
  if (engine_isKeyPressed(KEY_EQUAL)) {
    g_fpsIndex = (g_fpsIndex == COUNT(FPS) - 1) ? 0 : g_fpsIndex + 1;
  }
  SetTargetFPS(FPS[g_fpsIndex]);
}
#endif

// --- Game functions ---

bool game_load(void) {
  if (game_log != nullptr) {
    LOG_ERROR(game_log, "Game already loaded");
    return false;
  }
  game_log = log_create(&LOG_CONFIG_GAME);
  if (game_log == nullptr) {
    LOG_ERROR(game_log, "Failed to create log");
    return false;
  }

  double start = GetTime();
  GAME_TRY(asset_load());
  GAME_TRY(maze_init());
  GAME_TRY(asset_initPlayer());
  GAME_TRY(asset_initCreatures());

  LOG_INFO(game_log, "Game loading took %f seconds", GetTime() - start);
  return true;
}

void game_update(float frameTime) {
  float slop = BASE_SLOP * (frameTime / BASE_DT);
  slop       = fminf(fmaxf(slop, MIN_SLOP), MAX_SLOP);

#ifndef NDEBUG
  checkFPSKeys();
#endif

  LOG_TRACE(game_log, "Slop: %f", slop);
  draw_updateCreatures(frameTime, slop);
  draw_updatePlayer(frameTime, slop);
  maze_update(frameTime);
}

void game_draw(void) {
  maze_draw();
  draw_player();
  draw_creatures();
  draw_interface();
#ifndef NDEBUG
  debug_drawOverlay();
#endif
}

void game_unload(void) {
  asset_shutdownCreatures();
  asset_shutdownPlayer();
  maze_shutdown();
  asset_unload();
  log_destroy(&game_log);
}

int game_getLevel(void) { return g_level; }

void game_over(void) {
  player_totalReset();
  creature_reset();
  maze_reset();
  g_level = 1;
}

void game_nextLevel(void) {
  player_reset();
  creature_reset();
  maze_reset();
  g_level += 1;
}

void game_playerDead(void) {
  player_restart();
  creature_reset();
}
