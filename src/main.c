#include <raylib.h>

#include <engine/engine.h>
#include <game/game.h>
#include <log/log.h>

// --- Constants ---

static const char* WINDOW_TITLE   = "Mythic Dash";
static const int   ORG_SCR_WIDTH  = 480;  // Base canvas size
static const int   ORG_SCR_HEIGHT = 270;

static const log_Config LOG_CONFIG = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "MAIN"
};

// --- Main ---

int main(void) {
  log_Log* log = log_create(&LOG_CONFIG);
  if (log == nullptr) {
    LOG_ERROR(log, "Failed to create log");
    return 1;
  }

  if (!engine_init(ORG_SCR_WIDTH, ORG_SCR_HEIGHT, WINDOW_TITLE, 0, false, LOG_LEVEL_DEBUG)) {
    LOG_ERROR(log, "Failed to initialise engine");
    return 1;
  }

  LOG_INFO(log, "Loading game...");
  if (!game_load()) {
    LOG_ERROR(log, "Failed to load game");
    return 1;
  }
  LOG_INFO(log, "Game loaded");

  g_accumulator   = 0.0;
  double previous = engine_getTime();
  while (!engine_shouldClose()) {
    game_input();

    double now   = engine_getTime();
    double delta = now - previous;
    previous     = now;
    if (game_getDifficulty() == DIFFICULTY_ARCADE) {
      g_accumulator += delta;
      while (g_accumulator >= FRAME_TIME) {
        game_update(FRAME_TIME);
        g_accumulator -= FRAME_TIME;
      }
    } else {
      game_update(delta);
    }

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
