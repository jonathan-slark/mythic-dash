#include "engine.h"

#include "../log/log.h"

#include <assert.h>
#include <raylib.h>

// --- Macros ---

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// --- Constants ---

static const LogConfig LOG_CONFIG = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "ENG "
};

static const LogConfig LOG_CONFIG_RAYLIB = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = false,
  .subsystem     = "RAY "
};

// --- Global state ---

EngineScreenState gEngineScreenState;
static Log*       gEngineLog = nullptr;
static Log*       gRaylibLog = nullptr;

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

// --- Engine functions ---

void engine_init(int orgScreenWidth, int orgScreenHeight, const char* title) {
  gEngineLog = log_create(&LOG_CONFIG);
  gRaylibLog = log_create(&LOG_CONFIG_RAYLIB);

  LOG_INFO(gEngineLog, "Initializing engine...");

  SetTraceLogLevel(LOG_DEBUG);
  SetTraceLogCallback(raylibLog);

  InitWindow(0, 0, title);

  int scrWidth       = GetScreenWidth();
  int scrHeight      = GetScreenHeight();
  int scrRefreshRate = GetMonitorRefreshRate(GetCurrentMonitor());
  int scrScale       = MIN(scrWidth / orgScreenWidth, scrHeight / orgScreenHeight);

  gEngineScreenState = (EngineScreenState) {
    .width       = scrWidth,
    .height      = scrHeight,
    .refreshRate = scrRefreshRate,
    .scale       = scrScale,
  };

  LOG_INFO(gEngineLog, "Screen state: %d %d %d %d", scrWidth, scrHeight, scrRefreshRate, scrScale);

  SetTargetFPS(scrRefreshRate);
  ToggleBorderlessWindowed();
  HideCursor();
}

void engine_shutdown(void) {
  LOG_INFO(gEngineLog, "Shutting down engine...");
  CloseWindow();
  log_destroy(&gRaylibLog);
  log_destroy(&gEngineLog);
}
