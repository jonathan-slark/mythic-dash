#include "game.h"
#include "log/log.h"

#include <raylib.h>

// --- Macros ---

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// --- Types ---

typedef struct ScrState {
  int scrWidth;
  int scrHeight;
  int scrRefreshRate;
  int scrScale;
} ScrState;

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

ScrState    gScrState;
static Log* gRaylibLog = nullptr;

// --- Helper functions ---

static void raylibLog(int msgType, const char* text, va_list args) {
  LogLevel level;
  switch (msgType) {
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
  log_destroy(&gRaylibLog);
  log_destroy(&log);

  return 0;
}
