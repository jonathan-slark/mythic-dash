#include "game.h"
#include <assert.h>
#include <math.h>
#include <raylib.h>
#include "../engine/engine.h"
#include "../log/log.h"
#include "game/internal.h"
#include "internal.h"

// --- Macros ---

#define COUNT(array) (sizeof(array) / sizeof(array[0]))

// --- Types ---

typedef struct AnimData {
  int   row;
  int   startCol;
  int   frameCount;
  float frameTime;
} AnimData;

// --- Constants ---

static const char       FILE_BACKGROUND[] = "../../asset/gfx/background.png";
static const char       FILE_SPRITES[]    = "../../asset/gfx/sprites.png";
static const char       FILE_FONT[]       = "../../asset/gfx/font.png";

static const log_Config LOG_CONFIG_GAME   = {
    .minLevel      = LOG_LEVEL_DEBUG,
    .useColours    = true,
    .showTimestamp = true,
    .showFileLine  = true,
    .subsystem     = "GAME"
};

#ifndef NDEBUG
static const float FPS[] = { 15, 30, 60, 0 };
#endif

const float           BASE_SLOP               = 0.25f;
const float           BASE_DT                 = (1.0f / 144.0f);
const float           MIN_SLOP                = 0.05f;
const float           MAX_SLOP                = 0.5f;
const float           OVERLAP_EPSILON         = 1e-5f;

static const float    FRAME_TIME              = (1.0f / 12.0f);

static const Vector2  PLAYER_OFFSET           = { 0.0f, 0.0f };
static const AnimData PLAYER_ANIMS[DIR_COUNT] = {
  [DIR_UP]    = { 0, 0, 3, FRAME_TIME },
  [DIR_RIGHT] = { 1, 0, 3, FRAME_TIME },
  [DIR_DOWN]  = { 2, 0, 3, FRAME_TIME },
  [DIR_LEFT]  = { 3, 0, 3, FRAME_TIME }
};

static const Vector2 GHOST_OFFSETS[GHOST_COUNT] = {
  {  48.0f, 0.0f },
  {  80.0f, 0.0f },
  { 112.0f, 0.0f },
  { 144.0f, 0.0f }
};

// clang-format off
static const AnimData GHOST_ANIMS[GHOST_COUNT][DIR_COUNT] = {
  {
    [DIR_UP]    = { 0, 3, 2, FRAME_TIME },
    [DIR_RIGHT] = { 1, 3, 2, FRAME_TIME },
    [DIR_DOWN]  = { 2, 3, 2, FRAME_TIME },
    [DIR_LEFT]  = { 3, 3, 2, FRAME_TIME }
  },
  {
    [DIR_UP]    = { 0, 5, 2, FRAME_TIME },
    [DIR_RIGHT] = { 1, 5, 2, FRAME_TIME },
    [DIR_DOWN]  = { 2, 5, 2, FRAME_TIME },
    [DIR_LEFT]  = { 3, 5, 2, FRAME_TIME }
  },
  {
    [DIR_UP]    = { 0, 7, 2, FRAME_TIME },
    [DIR_RIGHT] = { 1, 7, 2, FRAME_TIME },
    [DIR_DOWN]  = { 2, 7, 2, FRAME_TIME },
    [DIR_LEFT]  = { 3, 7, 2, FRAME_TIME }
  },
  {
    [DIR_UP]    = { 0, 9, 2, FRAME_TIME },
    [DIR_RIGHT] = { 1, 9, 2, FRAME_TIME },
    [DIR_DOWN]  = { 2, 9, 2, FRAME_TIME },
    [DIR_LEFT]  = { 3, 9, 2, FRAME_TIME }
  }
};
// clang-format on

// --- Global state ---

log_Log*               game__log;
static engine_Texture* g_background;
static engine_Texture* g_sprites;
static engine_Sprite*  g_playerSprite;
static engine_Anim*    g_playerAnim[DIR_COUNT];
static engine_Sprite*  g_ghostSprites[GHOST_COUNT];
static engine_Anim*    g_ghostAnims[GHOST_COUNT][DIR_COUNT];
static engine_Font*    g_font;
#ifndef NDEBUG
static size_t g_fpsIndex = COUNT(FPS) - 1;
#endif

// --- Helper functions ---

#ifndef NDEBUG
static void checkFPSKeys(void) {
  if (engine_isKeyPressed(KEY_MINUS)) {
    g_fpsIndex = (g_fpsIndex == 0) ? COUNT(FPS) - 1 : g_fpsIndex - 1;
  }
  if (engine_isKeyPressed(KEY_EQUAL)) {
    g_fpsIndex = (g_fpsIndex == COUNT(FPS) - 1) ? 0 : g_fpsIndex + 1;
  }
  SetTargetFPS(FPS[g_fpsIndex]);
}
#endif

static bool loadAssets(void) {
  GAME_TRY(g_background = engine_textureLoad(FILE_BACKGROUND));
  GAME_TRY(g_sprites = engine_textureLoad(FILE_SPRITES));
  GAME_TRY(g_font = engine_fontLoad(FILE_FONT, 8, 8, 33, 126, 1));
  return true;
}

static bool initPlayer(void) {
  GAME_TRY(player_init());
  GAME_TRY(
      g_playerSprite = engine_createSprite(
          POS_ADJUST(player_getPos()), (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, PLAYER_OFFSET
      )
  );
  for (int i = 0; i < DIR_COUNT; i++) {
    GAME_TRY(
        g_playerAnim[i] = engine_createAnim(
            g_playerSprite,
            PLAYER_ANIMS[i].row,
            PLAYER_ANIMS[i].startCol,
            PLAYER_ANIMS[i].frameCount,
            PLAYER_ANIMS[i].frameTime
        )
    );
  }
  return true;
}

static bool initGhosts(void) {
  GAME_TRY(ghost_init());
  for (int i = 0; i < GHOST_COUNT; i++) {
    GAME_TRY(
        g_ghostSprites[i] = engine_createSprite(
            POS_ADJUST(ghost_getPos(i)), (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, GHOST_OFFSETS[i]
        )
    );
    for (int j = 0; j < DIR_COUNT; j++) {
      GAME_TRY(
          g_ghostAnims[i][j] = engine_createAnim(
              g_ghostSprites[i],
              GHOST_ANIMS[i][j].row,
              GHOST_ANIMS[i][j].startCol,
              GHOST_ANIMS[i][j].frameCount,
              GHOST_ANIMS[i][j].frameTime
          )
      );
    }
  }
  return true;
}

static void updatePlayer(float frameTime, float slop) {
  player_update(frameTime, slop);
  engine_spriteSetPos(g_playerSprite, POS_ADJUST(player_getPos()));
  if (player_isMoving()) {
    engine_updateAnim(g_playerAnim[player_getDir()], frameTime);
  }
}

static void updateGhosts(float frameTime, float slop) {
  ghost_update(frameTime, slop);
  for (int i = 0; i < GHOST_COUNT; i++) {
    engine_spriteSetPos(g_ghostSprites[i], POS_ADJUST(ghost_getPos(i)));
    engine_updateAnim(g_ghostAnims[i][ghost_getDir(i)], frameTime);
  }
}

static void unloadPlayer(void) {
  player_shutdown();
  engine_destroySprite(&g_playerSprite);
  for (int i = 0; i < DIR_COUNT; i++) {
    engine_destroyAnim(&g_playerAnim[i]);
  }
}

static void unloadGhosts(void) {
  ghost_shutdown();
  for (int i = 0; i < GHOST_COUNT; i++) {
    engine_destroySprite(&g_ghostSprites[i]);
    for (int j = 0; j < DIR_COUNT; j++) {
      engine_destroyAnim(&g_ghostAnims[i][j]);
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
  GAME_TRY(loadAssets());
  maze_init();
  GAME_TRY(initPlayer());
  GAME_TRY(initGhosts());

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
  updatePlayer(frameTime, slop);
  updateGhosts(frameTime, slop);
}

void game_draw(void) {
  engine_drawBackground(g_background);
  engine_drawSprite(g_sprites, g_playerSprite);
  for (int i = 0; i < GHOST_COUNT; i++) {
    engine_drawSprite(g_sprites, g_ghostSprites[i]);
  }
#ifndef NDEBUG
  debug_drawOverlay();
#endif
}

void game_unload(void) {
  unloadGhosts();
  unloadPlayer();
  engine_fontUnload(&g_font);
  engine_textureUnload(&g_sprites);
  engine_textureUnload(&g_background);
  log_destroy(&game__log);
}
