#include "game.h"
#include <assert.h>
#include <math.h>
#include <raylib.h>
#include "../engine/engine.h"
#include "../log/log.h"
#include "internal.h"

// --- Macros ---

#define COUNT(array) (sizeof(array) / sizeof(array[0]))

// --- Types ---

typedef struct game__Anim {
  engine_Sprite* frames;
  size_t         frameCount;
  size_t         currentFrame;
  float          frameTime;
  float          timeAccumulator;
} game__Anim;

// --- Constants ---

static const char       FILE_BACKGROUND[] = "../../asset/gfx/background.png";
static const char       FILE_SPRITES[]    = "../../asset/gfx/sprites.png";
static const char       FILE_FONT[]       = "../../asset/gfx/font.png";

static const log_Config LOG_CONFIG_GAME   = { .minLevel      = LOG_LEVEL_DEBUG,
                                              .useColours    = true,
                                              .showTimestamp = true,
                                              .showFileLine  = true,
                                              .subsystem     = "GAME" };

#ifndef NDEBUG
static const float FPS[] = { 15, 30, 60, 0 };
#endif

const float BASE_SLOP       = 0.25f;
const float BASE_DT         = (1.0f / 144.0f);  // Reference frame rate
const float MIN_SLOP        = 0.05f;
const float MAX_SLOP        = 0.5f;
const float OVERLAP_EPSILON = 1e-5f;

// --- Global state ---

log_Log*               game__log;
static engine_Texture* g_background;
static engine_Texture* g_sprites;
static engine_Font*    g_font;
static engine_Sprite   g_playerSprite = { .size = { ACTOR_SIZE, ACTOR_SIZE }, .offset = { 0, 0 } };
#ifndef NDEBUG
static size_t g_fpsIndex = COUNT(FPS) - 1;
#endif

// --- Helper functions ---

#ifndef NDEBUG
static void checkFPSKeys(void) {
  if (engine_isKeyPressed(KEY_MINUS)) {
    if (g_fpsIndex == 0) {
      g_fpsIndex = COUNT(FPS) - 1;
    } else {
      g_fpsIndex--;
    }
  }
  if (engine_isKeyPressed(KEY_EQUAL)) {
    if (g_fpsIndex == COUNT(FPS) - 1) {
      g_fpsIndex = 0;
    } else {
      g_fpsIndex++;
    }
  }
  SetTargetFPS(FPS[g_fpsIndex]);
}
#endif

void animUpdate(game__Anim* anim, float frameTime) {
  anim->timeAccumulator += frameTime;
  if (anim->timeAccumulator >= anim->frameTime) {
    anim->timeAccumulator = 0.0f;
    if (anim->currentFrame == anim->frameCount - 1) {
      anim->currentFrame = 0;
    } else {
      anim->currentFrame++;
    }
  }
}

// --- Game functions ---

bool game_load(void) {
  if (game__log != nullptr) {
    LOG_ERROR(game__log, "Game already loaded");
    return false;
  }
  game__log = log_create(&LOG_CONFIG_GAME);
  if (game__log == nullptr) {
    LOG_ERROR(game__log, "Failed to create log");
    return false;
  }

  double start = GetTime();

  GAME_TRY(g_background = engine_textureLoad(FILE_BACKGROUND));
  GAME_TRY(g_sprites = engine_textureLoad(FILE_SPRITES));
  GAME_TRY(g_font = engine_fontLoad(FILE_FONT, 8, 8, 33, 126, 1));

  maze_init();
  GAME_TRY(player_init());

  LOG_INFO(game__log, "Game loading took %f seconds", GetTime() - start);
  return true;
}

void game_update(float frameTime) {
  float slop = BASE_SLOP * (frameTime / BASE_DT);
  slop       = fminf(fmaxf(slop, MIN_SLOP), MAX_SLOP);

#ifndef NDEBUG
  checkFPSKeys();
#endif

  LOG_TRACE(game__log, "Slop: %f", slop);
  player_update(frameTime, slop);
}

void game_draw(void) {
  engine_drawBackground(g_background);

  g_playerSprite.position = POS_ADJUST(player_getPos());
  engine_drawSprite(g_sprites, &g_playerSprite);

#ifndef NDEBUG
  debug_drawOverlay();
#endif
}

void game_unload(void) {
  player_shutdown();
  engine_fontUnload(&g_font);
  engine_textureUnload(&g_sprites);
  engine_textureUnload(&g_background);
  log_destroy(&game__log);
}
