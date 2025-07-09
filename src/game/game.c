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

// --- Types ---

typedef enum game_GameState { GAME_BOOT, GAME_TITLE, GAME_MENU, GAME_RUN, GAME_OVER } game_GameState;

typedef struct Game {
  game_GameState state;
  int            level;
#ifndef NDEBUG
  size_t fpsIndex;
#endif
} Game;

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

static const float MASTER_VOLUME = 1.0f;

// --- Global state ---

log_Log*    game_log;
static Game g_game = { .state = GAME_BOOT, .level = 1, .fpsIndex = COUNT(FPS) - 1 };

// --- Helper functions ---

#ifndef NDEBUG
static void checkFPSKeys(void) {
  if (engine_isKeyPressed(KEY_MINUS)) {
    g_game.fpsIndex = (g_game.fpsIndex == 0) ? COUNT(FPS) - 1 : g_game.fpsIndex - 1;
  }
  if (engine_isKeyPressed(KEY_EQUAL)) {
    g_game.fpsIndex = (g_game.fpsIndex == COUNT(FPS) - 1) ? 0 : g_game.fpsIndex + 1;
  }
  SetTargetFPS(FPS[g_game.fpsIndex]);
}
#endif

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
  GAME_TRY(asset_load());
  GAME_TRY(maze_init());
  GAME_TRY(asset_initPlayer());
  GAME_TRY(asset_initCreatures());
  LOG_INFO(game_log, "Game loading took %f seconds", GetTime() - start);

  engine_playMusic(asset_getMusic());

  engine_showCursor();
  g_game.state = GAME_TITLE;
  return true;
}

void game_update(float frameTime) {
  float slop = BASE_SLOP * (frameTime / BASE_DT);
  slop       = fminf(fmaxf(slop, MIN_SLOP), MAX_SLOP);

  switch (g_game.state) {
    case GAME_BOOT: assert(false); break;
    case GAME_TITLE: menu_update(); break;
    case GAME_MENU: menu_update(); break;
    case GAME_RUN:
#ifndef NDEBUG
      checkFPSKeys();
#endif
      LOG_TRACE(game_log, "Slop: %f", slop);
      player_update(frameTime, slop);
      creature_update(frameTime, slop);
      draw_updateCreatures(frameTime, slop);
      draw_updatePlayer(frameTime, slop);
      maze_update(frameTime);
      engine_updateMusic(asset_getMusic(), frameTime);
      break;
    case GAME_OVER: break;
  }
}

void game_draw(void) {
  switch (g_game.state) {
    case GAME_BOOT: assert(false); break;
    case GAME_TITLE:
      draw_title();
      menu_draw();
      break;
    case GAME_MENU: menu_draw(); break;
    case GAME_RUN:
      maze_draw();
      draw_player();
      draw_nextLife();
      draw_creatures();
      draw_interface();

#ifndef NDEBUG
      debug_drawOverlay();
      break;
#endif
    case GAME_OVER: break;
  }
}

void game_unload(void) {
  engine_shutdownAudio();
  asset_shutdownCreatures();
  asset_shutdownPlayer();
  maze_shutdown();
  asset_unload();
  log_destroy(&game_log);
}

void game_new(void) {
  game_over();
  g_game.state = GAME_RUN;
}

void game_over(void) {
  player_totalReset();
  creature_reset();
  maze_reset();
  g_game.level = 1;
}

int game_getLevel(void) { return g_game.level; }

void game_nextLevel(void) {
  player_reset();
  creature_reset();
  maze_reset();
  g_game.level += 1;
}

void game_playerDead(void) {
  player_restart();
  creature_reset();
}
