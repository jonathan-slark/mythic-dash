#include "engine.h"
#include <assert.h>
#include <raylib.h>
#include "../log/log.h"
#include "internal.h"

// --- Macros ---

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// --- Constants ---

static const int        LOG_LEVEL_RAYLIB  = LOG_WARNING;

static const log_Config LOG_CONFIG_ENGINE = {.minLevel      = LOG_LEVEL_DEBUG,
                                             .useColours    = true,
                                             .showTimestamp = true,
                                             .showFileLine  = true,
                                             .subsystem     = "ENG "};

static const log_Config LOG_CONFIG_RAYLIB = {.minLevel      = LOG_LEVEL_DEBUG,
                                             .useColours    = true,
                                             .showTimestamp = true,
                                             .showFileLine  = false,
                                             .subsystem     = "RAY "};

// --- Global state ---

engine__ScreenState engine__screenState;
log_Log*            engine__log;
static log_Log*     g_raylibLog;

// --- Helper functions ---

static void raylibLog(int msgType, const char* text, va_list args) {
  assert(g_raylibLog != nullptr);
  assert(msgType >= 0 && msgType <= LOG_NONE);
  assert(text != nullptr);

  log_Level level;
  switch (msgType) {
    // TODO: is this correct?
    case LOG_TRACE: level = LOG_LEVEL_TRACE; break;
    case LOG_DEBUG: level = LOG_LEVEL_DEBUG; break;
    case LOG_INFO: level = LOG_LEVEL_INFO; break;
    case LOG_WARNING: level = LOG_LEVEL_WARN; break;
    case LOG_ERROR: level = LOG_LEVEL_ERROR; break;
    case LOG_FATAL: level = LOG_LEVEL_FATAL; break;
    default: level = LOG_LEVEL_INFO; break;
  }
  log_vmessage(g_raylibLog, level, __FILE__, __LINE__, true, text, args);
}

// --- Engine functions ---

bool engine_init(int nativeWidth, int nativeHeight, const char* title, int fps) {
  if (nativeWidth <= 0 || nativeHeight <= 0) {
    LOG_ERROR(engine__log, "Invalid screen size: %d %d", nativeWidth, nativeHeight);
    return false;
  }
  if (title == nullptr) {
    LOG_ERROR(engine__log, "Invalid title: nullptr");
    return false;
  }
  if (fps < 0) {
    LOG_ERROR(engine__log, "Invalid fps: %d", fps);
    return false;
  }

  engine__log = log_create(&LOG_CONFIG_ENGINE);
  if (engine__log == nullptr) {
    LOG_ERROR(engine__log, "Failed to create engine log");
    return false;
  }

  g_raylibLog = log_create(&LOG_CONFIG_RAYLIB);
  if (g_raylibLog == nullptr) {
    log_destroy(&engine__log);
    LOG_ERROR(engine__log, "Failed to create raylib log");
    return false;
  }

  LOG_INFO(engine__log, "Initializing engine...");

  SetTraceLogLevel(LOG_LEVEL_RAYLIB);
  SetTraceLogCallback(raylibLog);

  int flags = FLAG_BORDERLESS_WINDOWED_MODE | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED;
  if (fps == 0) {
    flags |= FLAG_VSYNC_HINT;
  }
  SetConfigFlags(flags);
  InitWindow(0, 0, title);

  int screenWidth       = GetScreenWidth();
  int screenHeight      = GetScreenHeight();
  int screenRefreshRate = GetMonitorRefreshRate(GetCurrentMonitor());
  int screenScale       = MIN(screenWidth / nativeWidth, screenHeight / nativeHeight);

  if (screenWidth <= 0 || screenHeight <= 0 || screenRefreshRate <= 0 || screenScale <= 0) {
    LOG_ERROR(engine__log, "Failed to get screen state");
    return false;
  }

  engine__screenState = (engine__ScreenState) {
      .width       = screenWidth,
      .height      = screenHeight,
      .refreshRate = screenRefreshRate,
      .scale       = screenScale,
  };

  LOG_INFO(engine__log, "Screen state: %dx%d @ %dHz, scale: %d", screenWidth, screenHeight, screenRefreshRate,
           screenScale);

  if (fps > 0) {
    SetTargetFPS(fps);
  } else {
    SetTargetFPS(screenRefreshRate);
  }

  HideCursor();

  LOG_INFO(engine__log, "Engine initialized");

  return true;
}

void engine_shutdown(void) {
  LOG_INFO(engine__log, "Shutting down engine...");

  CloseWindow();

  if (g_raylibLog != nullptr) {
    log_destroy(&g_raylibLog);
  }

  LOG_INFO(engine__log, "Engine shutdown");

  if (engine__log != nullptr) {
    log_destroy(&engine__log);
  }
}
