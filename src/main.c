#include "game.h"
#include "log/log.h"

#include <raylib.h>

// --- Types ---

typedef struct ScrState {
  int scrWidth;
  int scrHeight;
  int scrRefreshRate;
  int scrScale;
} ScrState;

// --- Constants ---

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

const char* WINDOW_TITLE   = "Maze Muncher";
const int   ORG_SCR_WIDTH  = 480;
const int   ORG_SCR_HEIGHT = 270;

// --- Global state ---

ScrState gScrState;

// --- Main ---

int main(void) {
  Log* log = log_create(&LOG_CONFIG);

#ifndef NDEBUG
  SetTraceLogLevel(LOG_DEBUG);
#else
  SetTraceLogLevel(LOG_WARNING);
#endif

  LOG_INFO(log, "Starting game...");

  InitWindow(0, 0, WINDOW_TITLE);

  int scrWidth       = GetScreenWidth();
  int scrHeight      = GetScreenHeight();
  int scrRefreshRate = GetMonitorRefreshRate(GetCurrentMonitor());
  int scrScale       = scrWidth / ORG_SCR_WIDTH;

  gScrState = (ScrState) {
    .scrWidth       = scrWidth,
    .scrHeight      = scrHeight,
    .scrRefreshRate = scrRefreshRate,
    .scrScale       = scrScale,
  };

  LOG_INFO(log, "Screen state: %d %d %d %d", scrWidth, scrHeight, scrRefreshRate, scrScale);

  SetTargetFPS(scrRefreshRate);
  ToggleBorderlessWindowed();
  HideCursor();

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
