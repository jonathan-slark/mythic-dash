#include "engine.h"

#include "../log/log.h"
#include "engine_internal.h"

#include <assert.h>
#include <raylib.h>

// --- Macros ---

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// --- Constants ---

static const log_Config LOG_CONFIG_ENGINE = {.minLevel = LOG_LEVEL_DEBUG,
                                             .useColours = true,
                                             .showTimestamp = true,
                                             .showFileLine = true,
                                             .subsystem = "ENG "};

static const log_Config LOG_CONFIG_RAYLIB = {.minLevel = LOG_LEVEL_DEBUG,
                                             .useColours = true,
                                             .showTimestamp = true,
                                             .showFileLine = false,
                                             .subsystem = "RAY "};

// --- Global state ---

engine__ScreenState engine__screenState;
log_Log *engine_log = nullptr;
static log_Log *g_raylibLog = nullptr;

// --- Helper functions ---

static void raylibLog(int msgType, const char *text, va_list args) {
  assert(g_raylibLog != nullptr);
  assert(msgType >= 0 && msgType <= LOG_NONE);
  assert(text != nullptr);

  log_Level level;
  switch (msgType) {
  // TODO: is this correct?
  case LOG_TRACE:
    level = LOG_LEVEL_TRACE;
    break;
  case LOG_DEBUG:
    level = LOG_LEVEL_DEBUG;
    break;
  case LOG_INFO:
    level = LOG_LEVEL_INFO;
    break;
  case LOG_WARNING:
    level = LOG_LEVEL_WARN;
    break;
  case LOG_ERROR:
    level = LOG_LEVEL_ERROR;
    break;
  case LOG_FATAL:
    level = LOG_LEVEL_FATAL;
    break;
  default:
    level = LOG_LEVEL_INFO;
    break;
  }
  log_vmessage(g_raylibLog, level, __FILE__, __LINE__, true, text, args);
}

// --- Engine functions ---

void engine_init(int orgScreenWidth, int orgScreenHeight, const char *title) {
  engine_log = log_create(&LOG_CONFIG_ENGINE);
  g_raylibLog = log_create(&LOG_CONFIG_RAYLIB);

  LOG_INFO(engine_log, "Initializing engine...");

  SetTraceLogLevel(LOG_DEBUG);
  SetTraceLogCallback(raylibLog);

  InitWindow(0, 0, title);

  int scrWidth = GetScreenWidth();
  int scrHeight = GetScreenHeight();
  int scrRefreshRate = GetMonitorRefreshRate(GetCurrentMonitor());
  int scrScale = MIN(scrWidth / orgScreenWidth, scrHeight / orgScreenHeight);

  engine__screenState = (engine__ScreenState){
      .width = scrWidth,
      .height = scrHeight,
      .refreshRate = scrRefreshRate,
      .scale = scrScale,
  };

  LOG_INFO(engine_log, "Screen state: %d %d %d %d", scrWidth, scrHeight,
           scrRefreshRate, scrScale);

  SetTargetFPS(scrRefreshRate);
  ToggleBorderlessWindowed();
  HideCursor();
}

bool engine_shouldClose(void) { return WindowShouldClose(); }

void engine_shutdown(void) {
  LOG_INFO(engine_log, "Shutting down engine...");
  CloseWindow();
  log_destroy(&g_raylibLog);
  log_destroy(&engine_log);
}
