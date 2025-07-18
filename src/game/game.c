#include <assert.h>
#include <engine/engine.h>
#include <game/game.h>
#include <log/log.h>
#include <math.h>
#include <raylib.h>
#include <stddef.h>
#include "asset/asset.h"
#include "creature/creature.h"
#include "debug/debug.h"
#include "draw/draw.h"
#include "internal.h"
#include "maze/maze.h"
#include "menu/menu.h"
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

const float BASE_SLOP       = 0.35f;
const float BASE_DT         = (1.0f / 144.0f);
const float MIN_SLOP        = 0.05f;
const float MAX_SLOP        = 2.5f;
const float OVERLAP_EPSILON = 2e-5f;

static const float MASTER_VOLUME = 1.0f;

// --- Global state ---

log_Log* game_log;
Game     g_game = { .state = GAME_BOOT, .level = 0, .fpsIndex = COUNT(FPS) - 1 };

// --- Helper functions ---

static void escapePressed(void) {
  switch (g_game.state) {
    case GAME_BOOT: assert(false); break;

    case GAME_TITLE:
    case GAME_MENU: menu_back(); break;

    case GAME_READY:
    case GAME_RUN:
    case GAME_PAUSE:
    case GAME_OVER: menu_open(MENU_CONTEXT_INGAME); break;
  }
}

static void spacePressed(void) {
  switch (g_game.state) {
    case GAME_BOOT: assert(false); break;

    case GAME_TITLE:
    case GAME_MENU: break;

    case GAME_PAUSE:
    case GAME_RUN:
#ifndef NDEBUG
      g_game.state = g_game.state == GAME_PAUSE ? GAME_RUN : GAME_PAUSE;
#endif
      break;

    case GAME_READY: g_game.state = GAME_RUN; break;

    case GAME_OVER: menu_open(MENU_CONTEXT_TITLE); break;
  }
}

static void checkKeys(void) {
  if (engine_isKeyPressed(KEY_SPACE)) spacePressed();
  if (engine_isKeyPressed(KEY_ESCAPE)) escapePressed();
}

static void drawGame(void) {
  maze_draw();
  draw_player();
  draw_nextLife();
  draw_creatures();
  draw_interface();
#ifndef NDEBUG
  debug_drawOverlay();
#endif
}

static void updateGame(float frameTime) {
  float slop = BASE_SLOP * (frameTime / BASE_DT);
  slop       = fminf(fmaxf(slop, MIN_SLOP), MAX_SLOP);
  LOG_TRACE(game_log, "Slop: %f", slop);

  player_update(frameTime, slop);
  creature_update(frameTime, slop);
  draw_updateCreatures(frameTime, slop);
  draw_updatePlayer(frameTime, slop);
  maze_update(frameTime);
  engine_updateMusic(asset_getMusic(), frameTime);
}

// --- Game functions ---

bool game_load(void) {
  assert(g_game.state == GAME_BOOT);

  double start = GetTime();

  if (game_log != nullptr) {
    LOG_ERROR(game_log, "Game already loaded");
    return false;
  }
  game_log = log_create(&LOG_CONFIG_GAME);
  if (game_log == nullptr) {
    LOG_ERROR(game_log, "Failed to create log");
    return false;
  }

  engine_initAudio(MASTER_VOLUME);
  GAME_TRY(maze_init());
  GAME_TRY(asset_load());  // Requires maze_init() for the tileset
  GAME_TRY(asset_initPlayer());
  GAME_TRY(asset_initCreatures());
  GAME_TRY(asset_initCursor());
  LOG_INFO(game_log, "Game loading took %f seconds", GetTime() - start);

  engine_playMusic(asset_getMusic());

  menu_open(MENU_CONTEXT_TITLE);
  return true;
}

void game_update(float frameTime) {
  checkKeys();
  switch (g_game.state) {
    case GAME_BOOT: assert(false); break;

    case GAME_TITLE:
    case GAME_MENU: menu_update(); break;

    case GAME_READY:
    case GAME_PAUSE:
    case GAME_OVER: checkFPSKeys(); break;

    case GAME_RUN:
      checkFPSKeys();
      updateGame(frameTime);
      break;
  }
}

void game_draw(void) {
  switch (g_game.state) {
    case GAME_BOOT: assert(false); break;
    case GAME_TITLE:
      draw_title();
      menu_draw();
      break;
    case GAME_MENU:
      drawGame();
      menu_draw();
      break;
    case GAME_READY:
      drawGame();
      draw_ready();
      break;
    case GAME_RUN:
    case GAME_PAUSE: drawGame(); break;
    case GAME_OVER:
      drawGame();
      draw_gameOver();
      break;
  }
}

void game_unload(void) {
  engine_shutdownAudio();
  asset_shutdownCursor();
  asset_shutdownCreatures();
  asset_shutdownPlayer();
  maze_shutdown();
  asset_unload();
  log_destroy(&game_log);
}

void game_new(void) {
  player_totalReset();
  creature_reset();
  maze_reset(g_game.level);
  draw_resetCreatures();
  draw_resetPlayer();
  g_game.level = 0;
  g_game.state = GAME_READY;
}

void game_over(void) { g_game.state = GAME_OVER; }

int game_getLevel(void) { return g_game.level; }

void game_nextLevel(void) {
  player_reset();
  creature_reset();
  maze_reset(g_game.level);
  g_game.level += 1;
  g_game.state  = GAME_READY;
  draw_resetCreatures();
}

void game_playerDead(void) {
  player_restart();
  creature_reset();
  g_game.state = GAME_READY;
}
