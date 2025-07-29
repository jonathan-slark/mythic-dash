#include <assert.h>
#include <engine/engine.h>
#include <game/game.h>
#include <log/log.h>
#include <math.h>
#include <raylib.h>
#include <stddef.h>
#include "asset/asset.h"
#include "audio/audio.h"
#include "creature/creature.h"
#include "debug/debug.h"
#include "draw/draw.h"
#include "input/input.h"
#include "internal.h"
#include "maze/maze.h"
#include "menu/menu.h"
#include "player/player.h"
#include "scores/scores.h"

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

static const float MASTER_VOLUME                        = 1.0f;
static const char  SCREENSHOT_FILE[]                    = "screenshot.png";
static const char* DIFFICULTY_STRINGS[DIFFICULTY_COUNT] = { "Easy", "Normal", "Arcade Mode" };

// --- Global state ---

log_Log* game_log;
double   g_accumulator;
Game     g_game = { .state = GAME_BOOT, .fpsIndex = COUNT(FPS) - 1 };

// --- Helper functions ---

static void updateMusic(double frameTime) { engine_updateMusic(asset_getMusic(g_game.musicTrack), frameTime); }

static void escapePressed(void) {
  switch (g_game.state) {
    case GAME_BOOT: assert(false); break;

    case GAME_TITLE:
    case GAME_MENU: menu_back(); break;

    case GAME_START:
    case GAME_DEAD:
    case GAME_RUN:
    case GAME_PAUSE:
    case GAME_OVER:
    case GAME_LEVELCLEAR:
    case GAME_WON: menu_open(MENU_CONTEXT_INGAME); break;
  }
}

static void readyCommon(void) {
  g_game.state = GAME_RUN;
  if (g_game.isMusicPaused) {
    audio_resumeMusic();
  } else {
    audio_startMusic();
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

    case GAME_START:
      readyCommon();
      player_ready();
      break;

    case GAME_DEAD:
      readyCommon();
      player_onResume();
      break;

    case GAME_LEVELCLEAR:
      g_game.state = GAME_START;
      game_nextLevel();
      break;

    case GAME_OVER:
    case GAME_WON: menu_open(MENU_CONTEXT_TITLE); break;
  }
}

static void checkKeys(void) {
  if (input_isKeyPressed(INPUT_SPACE)) spacePressed();
  if (input_isKeyPressed(INPUT_ESCAPE)) escapePressed();
  if (input_isKeyPressed(INPUT_S)) TakeScreenshot(SCREENSHOT_FILE);
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

static void updateGame(double frameTime) {
  float slop = BASE_SLOP * (frameTime / BASE_DT);
  slop       = fminf(fmaxf(slop, MIN_SLOP), MAX_SLOP);
  LOG_TRACE(game_log, "Slop: %f", slop);

  player_update(frameTime, slop);
  creature_update(frameTime, slop);
  draw_updateCreatures(frameTime, slop);
  draw_updatePlayer(frameTime, slop);
  maze_update(frameTime);
}

static void gameWon(void) { g_game.state = GAME_WON; }

static void startGame(int level) {
  LOG_INFO(game_log, "Starting new game, difficulty: %s", DIFFICULTY_STRINGS[g_game.startDifficulty]);
  g_game.difficulty = g_game.startDifficulty;
  g_game.level      = level;
  g_game.state      = GAME_START;
  g_game.musicTrack = 0;
  player_totalReset();
  creature_reset();
  maze_reset(g_game.level);
  draw_resetCreatures();
  draw_resetPlayer();
  debug_reset();
}

// --- Game functions ---

bool game_load(void) {
  assert(g_game.state == GAME_BOOT);

  double start = engine_getTime();

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
  scores_load();
  LOG_INFO(game_log, "Game loading took %f seconds", engine_getTime() - start);

  menu_open(MENU_CONTEXT_TITLE);
  return true;
}

void game_new(void) { startGame(0); }

void game_continue(void) {
  int level = player_getContinue(g_game.startDifficulty);
  if (level > 0) startGame(level);
}

void game_input(void) { input_update(); }

void game_update(double frameTime) {
  checkKeys();
  switch (g_game.state) {
    case GAME_BOOT: assert(false); break;

    case GAME_TITLE:
    case GAME_MENU: menu_update(); break;

    case GAME_START:
    case GAME_DEAD:
    case GAME_PAUSE:
    case GAME_LEVELCLEAR:
    case GAME_OVER:
    case GAME_WON: break;

    case GAME_RUN:
      checkFPSKeys();
      updateGame(frameTime);
      updateMusic(frameTime);
      break;
  }
  input_flush();
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

    case GAME_START:
    case GAME_DEAD:
      drawGame();
      draw_ready();
      break;

    case GAME_RUN:
    case GAME_PAUSE: drawGame(); break;

    case GAME_LEVELCLEAR:
      drawGame();
      draw_levelClear();
      break;

    case GAME_OVER:
      drawGame();
      draw_gameOver();
      break;

    case GAME_WON:
      drawGame();
      draw_gameWon();
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

void game_setEasy(void) { g_game.startDifficulty = DIFFICULTY_EASY; }

void game_setNormal(void) { g_game.startDifficulty = DIFFICULTY_NORMAL; }

void game_setArcade(void) { g_game.startDifficulty = DIFFICULTY_ARCADE; }

game_Difficulty game_getDifficulty(void) { return g_game.difficulty; }

void game_over(void) {
  audio_stopMusic();
  g_game.state = GAME_OVER;
}

int game_getLevel(void) { return g_game.level; }

int game_getStartDifficulty(void) { return g_game.startDifficulty; }

void game_levelClear(void) {
  scores_save();
  g_game.state = GAME_LEVELCLEAR;
}

void game_nextLevel(void) {
  if (g_game.level == LEVEL_COUNT - 1) {
    gameWon();
  } else {
    g_game.level += 1;
    g_game.state  = GAME_START;
    audio_stopMusic();
    player_reset();
    creature_reset();
    maze_reset(g_game.level);
    draw_resetPlayer();
    draw_resetCreatures();
  }
}

void game_playerDead(void) {
  g_game.state = GAME_DEAD;
  audio_pauseMusic();
  player_restart();
  creature_reset();
}
