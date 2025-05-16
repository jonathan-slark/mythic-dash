#include "game.h"
#include "log/log.h"
#include "raylib.h"

const LogConfig LOG_CONFIG = {
#ifndef NDEBUG
  .minLevel = LOG_LEVEL_DEBUG,
#else
  .minLevel = LOG_LEVEL_WARN,
#endif
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "MAIN"
};

int main(void) {
  Log*      log          = log_create(&LOG_CONFIG);
  const int screenWidth  = 800;
  const int screenHeight = 600;

#ifndef NDEBUG
  SetTraceLogLevel(LOG_DEBUG);
#else
  SetTraceLogLevel(LOG_WARNING);
#endif

  LOG_INFO(log, "Starting game...");

  InitWindow(screenWidth, screenHeight, "Maze Muncher");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

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
  CloseWindow();

  LOG_INFO(log, "Game closed");
  log_destroy(&log);

  return 0;
}
