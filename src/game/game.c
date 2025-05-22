#include "game.h"
#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include "../engine/engine.h"
#include "../log/log.h"
#include "game_internal.h"

// --- Constants ---

static const char       FILE_BACKGROUND[] = "../../asset/gfx/background.png";
static const char       FILE_SPRITES[]    = "../../asset/gfx/sprites.png";
static const char       FILE_FONT[]       = "../../asset/gfx/font.png";

static const log_Config LOG_CONFIG_GAME   = {.minLevel      = LOG_LEVEL_TRACE,
                                             .useColours    = true,
                                             .showTimestamp = true,
                                             .showFileLine  = true,
                                             .subsystem     = "GAME"};

// --- Global state ---

log_Log*               game__log;
bool                   game__isOverlayEnabled = false;
static engine_Texture* g_background;
static engine_Texture* g_sprites;
static engine_Font*    g_font;
static engine_Sprite   g_playerSprite = {.size = {ACTOR_SIZE, ACTOR_SIZE}, .offset = {0, 0}};

// --- Game functions ---

bool game_load(void) {
  game__log = log_create(&LOG_CONFIG_GAME);
  if (game__log == nullptr) {
    LOG_ERROR(game__log, "Failed to create log");
    return false;
  }

  double start = GetTime();

  GAME_TRY(g_background = engine_textureLoad(FILE_BACKGROUND));
  GAME_TRY(g_sprites = engine_textureLoad(FILE_SPRITES));
  GAME_TRY(g_font = engine_fontLoad(FILE_FONT, 8, 8, 33, 126, 1));

  GAME_TRY(game__mazeInit());
  game__playerInit();

  LOG_INFO(game__log, "Game loading took %f seconds", GetTime() - start);
  return true;
}

void game_update(float frameTime) { game__playerUpdate(frameTime); }

void game_draw(void) {
  engine_drawBackground(g_background);

  g_playerSprite.position = POS_ADJUST(game__playerGetPos());
  engine_drawSprite(g_sprites, &g_playerSprite);

#ifndef NDEBUG
  if (game__isOverlayEnabled) {
    game__playerOverlay();
    game__mazeOverlay();
  }
#endif
}

void game_unload(void) {
  game__mazeUninit();
  engine_fontUnload(&g_font);
  engine_textureUnload(&g_sprites);
  engine_textureUnload(&g_background);
  log_destroy(&game__log);
}
