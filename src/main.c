#include "engine/engine.h"
#include "game/game.h"
#include "log/log.h"

#include <raylib.h>

// --- Constants ---

static const char *WINDOW_TITLE = "Maze Muncher";
static const int ORG_SCR_WIDTH = 480;
static const int ORG_SCR_HEIGHT = 270;

static const log_Config LOG_CONFIG = {.minLevel = LOG_LEVEL_DEBUG,
                                      .useColours = true,
                                      .showTimestamp = true,
                                      .showFileLine = true,
                                      .subsystem = "MAIN"};

// --- Main ---

int main(void) {
  log_Log *log = log_create(&LOG_CONFIG);

  engine_init(ORG_SCR_WIDTH, ORG_SCR_HEIGHT, WINDOW_TITLE);

  LOG_INFO(log, "Loading game...");
  game_load();
  LOG_INFO(log, "Game loaded");

  while (!engine_shouldClose()) {
    game_update();
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
