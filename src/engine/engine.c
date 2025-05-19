#include "engine.h"

#include "../log/log.h"
#include "engine_internal.h"

#include <assert.h>
#include <raylib.h>

// --- Macros ---

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// --- Constants ---

const int LOG_LEVEL_RAYLIB = LOG_WARNING;

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
log_Log* engine_log = nullptr;
static log_Log* g_raylibLog = nullptr;

// --- Helper functions ---

static void raylibLog(int msgType, const char* text, va_list args) {
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

bool engine_init(int orgScreenWidth, int orgScreenHeight, const char* title) {
  if (orgScreenWidth <= 0 || orgScreenHeight <= 0) {
    LOG_ERROR(engine_log, "Invalid screen size: %d %d", orgScreenWidth, orgScreenHeight);
    return false;
  }
  if (title == nullptr) {
    LOG_ERROR(engine_log, "Invalid title: nullptr");
    return false;
  }

  engine_log = log_create(&LOG_CONFIG_ENGINE);
  if (engine_log == nullptr) {
    LOG_ERROR(engine_log, "Failed to create engine log");
    return false;
  }

  g_raylibLog = log_create(&LOG_CONFIG_RAYLIB);
  if (g_raylibLog == nullptr) {
    log_destroy(&engine_log);
    LOG_ERROR(engine_log, "Failed to create raylib log");
    return false;
  }

  LOG_INFO(engine_log, "Initializing engine...");

  SetTraceLogLevel(LOG_LEVEL_RAYLIB);
  SetTraceLogCallback(raylibLog);

  InitWindow(0, 0, title);

  int scrWidth = GetScreenWidth();
  int scrHeight = GetScreenHeight();
  int scrRefreshRate = GetMonitorRefreshRate(GetCurrentMonitor());
  int scrScale = MIN(scrWidth / orgScreenWidth, scrHeight / orgScreenHeight);

  if (scrWidth <= 0 || scrHeight <= 0 || scrRefreshRate <= 0 || scrScale <= 0) {
    LOG_ERROR(engine_log, "Failed to get screen state");
    return false;
  }

  engine__screenState = (engine__ScreenState){
      .width = scrWidth,
      .height = scrHeight,
      .refreshRate = scrRefreshRate,
      .scale = scrScale,
  };

  LOG_INFO(engine_log, "Screen state: %d %d %d %d", scrWidth, scrHeight, scrRefreshRate, scrScale);

  SetTargetFPS(scrRefreshRate);
  ToggleBorderlessWindowed();
  HideCursor();

  LOG_INFO(engine_log, "Engine initialized");

  return true;
}

bool engine_shouldClose(void) {
  return WindowShouldClose();
}

void engine_shutdown(void) {
  LOG_INFO(engine_log, "Shutting down engine...");

  CloseWindow();

  if (g_raylibLog != nullptr) {
    log_destroy(&g_raylibLog);
  }

  LOG_INFO(engine_log, "Engine shutdown");

  if (engine_log != nullptr) {
    log_destroy(&engine_log);
  }
}
