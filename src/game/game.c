#include "game.h"
#include <assert.h>
#include <engine/engine.h>
#include <game/game.h>
#include <log/log.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include "asset.h"

// --- Types ---

typedef struct game__Assets {
  engine_Texture* creatureSpriteSheet;
  engine_Texture* playerSpriteSheet;
  engine_Sprite*  playerSprite;
  engine_Anim*    playerAnim[DIR_COUNT];
  engine_Sprite*  creatureSprites[CREATURE_COUNT];
  engine_Anim*    creatureAnims[CREATURE_COUNT][DIR_COUNT];
  engine_Font*    font;
} game__Assets;

// --- Constants ---

static const log_Config LOG_CONFIG_GAME = {
  .minLevel      = LOG_LEVEL_DEBUG,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "GAME"
};

#ifndef NDEBUG
static const float FPS[] = { 15, 30, 60, 0 };
#endif

const float BASE_SLOP       = 0.35f;
const float BASE_DT         = (1.0f / 144.0f);
const float MIN_SLOP        = 0.05f;
const float MAX_SLOP        = 0.7f;
const float OVERLAP_EPSILON = 1e-5f;

// --- Global state ---

log_Log*            game__log;
static game__Assets g_assets;
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
  GAME_TRY(g_assets.creatureSpriteSheet = engine_textureLoad(FILE_CREATURES));
  GAME_TRY(g_assets.playerSpriteSheet = engine_textureLoad(FILE_PLAYER));
  GAME_TRY(g_assets.font = engine_fontLoad(FILE_FONT, 8, 8, 33, 126, 1));
  return true;
}

static void unloadAssets(void) {
  engine_fontUnload(&g_assets.font);
  engine_textureUnload(&g_assets.playerSpriteSheet);
  engine_textureUnload(&g_assets.creatureSpriteSheet);
}

static bool initPlayer(void) {
  GAME_TRY(player_init());
  GAME_TRY(
      g_assets.playerSprite = engine_createSprite(
          POS_ADJUST(player_getPos()), (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, (Vector2) { 0.0f, 0.0f }
      )
  );
  for (int i = 0; i < DIR_COUNT; i++) {
    GAME_TRY(
        g_assets.playerAnim[i] = engine_createAnim(
            g_assets.playerSprite,
            PLAYER_DATA.animData[i].row,
            PLAYER_DATA.animData[i].startCol,
            PLAYER_DATA.animData[i].frameCount,
            PLAYER_DATA.animData[i].frameTime,
            PLAYER_DATA.inset
        )
    );
  }
  return true;
}

static bool initGhosts(void) {
  GAME_TRY(ghost_init());
  for (int i = 0; i < CREATURE_COUNT; i++) {
    GAME_TRY(
        g_assets.creatureSprites[i] =
            engine_createSprite(POS_ADJUST(ghost_getPos(i)), CREATURE_DATA[i].size, (Vector2) { 0.0f, 0.0f })
    );
    for (int j = 0; j < DIR_COUNT; j++) {
      GAME_TRY(
          g_assets.creatureAnims[i][j] = engine_createAnim(
              g_assets.creatureSprites[i],
              CREATURE_DATA[i].animData[j].row,
              CREATURE_DATA[i].animData[j].startCol,
              CREATURE_DATA[i].animData[j].frameCount,
              CREATURE_DATA[i].animData[j].frameTime,
              CREATURE_DATA[i].inset
          )
      );
    }
  }
  return true;
}

static void updatePlayer(float frameTime, float slop) {
  player_update(frameTime, slop);
  engine_spriteSetPos(g_assets.playerSprite, POS_ADJUST(player_getPos()));
  if (player_isMoving()) {
    engine_updateAnim(g_assets.playerAnim[player_getDir()], frameTime);
  }
}

static void updateGhosts(float frameTime, float slop) {
  ghost_update(frameTime, slop);
  for (int i = 0; i < CREATURE_COUNT; i++) {
    Vector2 pos = Vector2Add(POS_ADJUST(ghost_getPos(i)), CREATURE_DATA[i].offset);
    engine_spriteSetPos(g_assets.creatureSprites[i], pos);
    engine_updateAnim(g_assets.creatureAnims[i][ghost_getDir(i)], frameTime);
  }
}

static void unloadPlayer(void) {
  player_shutdown();
  engine_destroySprite(&g_assets.playerSprite);
  for (int i = 0; i < DIR_COUNT; i++) {
    engine_destroyAnim(&g_assets.playerAnim[i]);
  }
}

static void unloadGhosts(void) {
  ghost_shutdown();
  for (int i = 0; i < CREATURE_COUNT; i++) {
    engine_destroySprite(&g_assets.creatureSprites[i]);
    for (int j = 0; j < DIR_COUNT; j++) {
      engine_destroyAnim(&g_assets.creatureAnims[i][j]);
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
  GAME_TRY(maze_init());
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
  maze_update(frameTime);
}

void game_draw(void) {
  maze_draw();
  engine_drawSprite(g_assets.playerSpriteSheet, g_assets.playerSprite);
  for (int i = 0; i < CREATURE_COUNT; i++) {
    engine_drawSprite(g_assets.creatureSpriteSheet, g_assets.creatureSprites[i]);
  }
#ifndef NDEBUG
  debug_drawOverlay();
#endif
}

void game_unload(void) {
  unloadGhosts();
  unloadPlayer();
  maze_shutdown();
  unloadAssets();
  log_destroy(&game__log);
}
