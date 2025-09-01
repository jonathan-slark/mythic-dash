#include <raylib.h>

#include <engine/engine.h>
#include <game/game.h>
#include <log/log.h>
#include "game/options/options.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// --- Constants ---

static const char* WINDOW_TITLE   = "Mythic Dash";
static const int   ORG_SCR_WIDTH  = 480;  // Base canvas size
static const int   ORG_SCR_HEIGHT = 270;

static const log_Config LOG_CONFIG = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "MAIN"
};

// --- Global state ---

double g_previousTime;

// --- Helper Functions ---

void mainLoop(void) {
  game_input();

  double now     = engine_getTime();
  double delta   = now - g_previousTime;
  g_previousTime = now;
  if (game_getDifficulty() == DIFFICULTY_ARCADE) {
    g_accumulator += delta;
    while (g_accumulator >= FRAME_TIME) {
      game_update(FRAME_TIME);
      g_accumulator -= FRAME_TIME;
    }
  } else {
    game_update(delta);
  }

  engine_beginFrame();
  engine_clearScreen(BLACK);
  game_draw();
  engine_endFrame();
}

// --- Main ---

int main(void) {
  log_Log* log = log_create(&LOG_CONFIG);
  if (log == nullptr) {
    LOG_FATAL(log, "Failed to create log");
    return 1;
  }

  options_load();
  int windowMode  = options_getWindowMode();
  int screenScale = options_getScreenScale();
  if (!engine_init(ORG_SCR_WIDTH, ORG_SCR_HEIGHT, WINDOW_TITLE, windowMode, screenScale, 0, LOG_LEVEL_INFO)) {
    LOG_FATAL(log, "Failed to initialise engine");
    return 1;
  }
  if (screenScale == -1) options_setScreenSacle(engine_getScale());

  LOG_INFO(log, "Loading game...");
  if (!game_load()) {
    LOG_FATAL(log, "Failed to load game");
    return 1;
  }
  LOG_INFO(log, "Game loaded");

  g_accumulator  = 0.0;
  g_previousTime = engine_getTime();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(mainLoop, 0, 1);
#else
  while (!engine_shouldClose()) {
    mainLoop();
  }
#endif

  LOG_INFO(log, "Closing game...");
  game_unload();
  LOG_INFO(log, "Game closed");

  engine_shutdown();

  log_destroy(&log);

  return 0;
}
