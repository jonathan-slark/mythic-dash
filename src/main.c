#include <raylib.h>

#include "engine/engine.h"
#include "game/game.h"
#include "log/log.h"

// --- Constants ---

static const char*      WINDOW_TITLE   = "Maze Muncher";
static const int        ORG_SCR_WIDTH  = 480;
static const int        ORG_SCR_HEIGHT = 270;

static const log_Config LOG_CONFIG     = { .minLevel      = LOG_LEVEL_DEBUG,
                                           .useColours    = true,
                                           .showTimestamp = true,
                                           .showFileLine  = true,
                                           .subsystem     = "MAIN" };

// --- Main ---

int main(void) {
  log_Log* log = log_create(&LOG_CONFIG);
  if (log == nullptr) {
    LOG_ERROR(log, "Failed to create log");
    return 1;
  }

  if (!engine_init(ORG_SCR_WIDTH, ORG_SCR_HEIGHT, WINDOW_TITLE, 0)) {
    LOG_ERROR(log, "Failed to initialise engine");
    return 1;
  }

  LOG_INFO(log, "Loading game...");
  if (!game_load()) {
    LOG_ERROR(log, "Failed to load game");
    return 1;
  }
  LOG_INFO(log, "Game loaded");

  while (!engine_shouldClose()) {
    float frameTime = engine_getFrameTime();
    game_update(frameTime);
    engine_beginFrame();
    engine_clearScreen(BLACK);
    game_draw();
    engine_endFrame();
  }

  LOG_INFO(log, "Closing game...");
  game_unload();
  LOG_INFO(log, "Game closed");

  engine_shutdown();

  log_destroy(&log);

  return 0;
}
