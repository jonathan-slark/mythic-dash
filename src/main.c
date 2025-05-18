#include "common.h"
#include "game.h"
#include "log/log.h"

#include <assert.h>
#include <raylib.h>

// --- Macros ---

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// --- Constants ---

const LogConfig LOG_CONFIG = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "MAIN"
};

const LogConfig LOG_CONFIG_RAYLIB = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = false,
  .subsystem     = "RAYL"
};

const char* WINDOW_TITLE   = "Maze Muncher";
const int   ORG_SCR_WIDTH  = 480;
const int   ORG_SCR_HEIGHT = 270;

// --- Global state ---

ScreenState gScreenState;
static Log* gRaylibLog = nullptr;

// --- Helper functions ---

static void raylibLog(int msgType, const char* text, va_list args) {
  assert(gRaylibLog != nullptr);
  assert(msgType >= 0 && msgType <= LOG_NONE);
  assert(text != nullptr);

  LogLevel level;
  switch (msgType) {
  // TODO: is this correct?
  case LOG_TRACE  : level = LOG_LEVEL_TRACE; break;
  case LOG_DEBUG  : level = LOG_LEVEL_DEBUG; break;
  case LOG_INFO   : level = LOG_LEVEL_INFO; break;
  case LOG_WARNING: level = LOG_LEVEL_WARN; break;
  case LOG_ERROR  : level = LOG_LEVEL_ERROR; break;
  case LOG_FATAL  : level = LOG_LEVEL_FATAL; break;
  default         : level = LOG_LEVEL_INFO; break;
  }
  log_vmessage(gRaylibLog, level, __FILE__, __LINE__, true, text, args);
}

// --- Main ---

int main(void) {
  Log* log   = log_create(&LOG_CONFIG);
  gRaylibLog = log_create(&LOG_CONFIG_RAYLIB);

  LOG_INFO(log, "Starting game...");

  SetTraceLogLevel(LOG_DEBUG);
  SetTraceLogCallback(raylibLog);

  InitWindow(0, 0, WINDOW_TITLE);

  int scrWidth       = GetScreenWidth();
  int scrHeight      = GetScreenHeight();
  int scrRefreshRate = GetMonitorRefreshRate(GetCurrentMonitor());
  int scrScale       = MIN(scrWidth / ORG_SCR_WIDTH, scrHeight / ORG_SCR_HEIGHT);

  gScreenState = (ScreenState) {
    .width       = scrWidth,
    .height      = scrHeight,
    .refreshRate = scrRefreshRate,
    .scale       = scrScale,
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
  log_destroy(&gRaylibLog);
  log_destroy(&log);

  return 0;
}
