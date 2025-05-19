#include "engine/engine.h"
#include "game/game.h"
#include "log/log.h"

#include <assert.h>
#include <raylib.h>

// --- Constants ---

static const char* WINDOW_TITLE   = "Maze Muncher";
static const int   ORG_SCR_WIDTH  = 480;
static const int   ORG_SCR_HEIGHT = 270;

static const LogConfig LOG_CONFIG = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "MAIN"
};

// --- Main ---

int main(void) {
  Log* log = log_create(&LOG_CONFIG);

  engine_init(ORG_SCR_WIDTH, ORG_SCR_HEIGHT, WINDOW_TITLE);

  LOG_INFO(log, "Loading game...");
  game_load();
  LOG_INFO(log, "Game loaded");

  while (!WindowShouldClose()) {
    game_update();
    BeginDrawing();
    ClearBackground(BLACK);
    game_draw();
    EndDrawing();
  }

  LOG_INFO(log, "Closing game...");
  game_unload();
  LOG_INFO(log, "Game closed");

  engine_shutdown();

  log_destroy(&log);

  return 0;
}
