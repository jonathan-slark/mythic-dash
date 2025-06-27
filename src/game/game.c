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

log_Log*     game__log;
game__Assets g_assets;
int          g_level = 1;
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
  GAME_TRY(g_assets.font = engine_fontLoad(FILE_FONT, 6, 10, 32, 127, 0, 2));
  GAME_TRY(g_assets.fontTiny = engine_fontLoad(FILE_FONT_TINY, 5, 7, 48, 57, 0, 0));
  return true;
}

static void unloadAssets(void) {
  engine_fontUnload(&g_assets.font);
  engine_textureUnload(&g_assets.playerSpriteSheet);
  engine_textureUnload(&g_assets.creatureSpriteSheet);
}

static bool initPlayer(void) {
  GAME_TRY(player_init());

  for (int i = 0; i < PLAYER_STATE_COUNT; i++) {
    GAME_TRY(
        g_assets.playerSprites[i] = engine_createSprite(
            POS_ADJUST(player_getPos()), (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, (Vector2) { 0.0f, 0.0f }
        )
    );
    for (int j = 0; j < DIR_COUNT; j++) {
      GAME_TRY(
          g_assets.playerAnim[i][j] = engine_createAnim(
              g_assets.playerSprites[i],
              PLAYER_DATA[i].animData[j].row,
              PLAYER_DATA[i].animData[j].startCol,
              PLAYER_DATA[i].animData[j].frameCount,
              PLAYER_DATA[i].animData[j].frameTime,
              PLAYER_DATA[i].inset,
              PLAYER_DATA[i].loop
          )
      );
    }
  }

  Vector2 offset = PLAYER_LIVES_OFFSET;
  for (int i = 0; i < PLAYER_MAX_LIVES; i++) {
    GAME_TRY(
        g_assets.playerLivesSprites[i] = engine_createSpriteFromSheet(
            offset,
            (Vector2) { ACTOR_SIZE, ACTOR_SIZE },
            PLAYER_DATA[PLAYER_NORMAL].animData[DIR_LEFT].row,
            PLAYER_DATA[PLAYER_NORMAL].animData[DIR_LEFT].startCol,
            PLAYER_DATA[PLAYER_NORMAL].inset
        )
    );
    offset.x += ACTOR_SIZE;
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
              CREATURE_DATA[i].inset,
              CREATURE_DATA[i].loop
          )
      );
    }
  }
  return true;
}

static void unloadPlayer(void) {
  player_shutdown();
  for (int i = 0; i < PLAYER_LIVES; i++) {
    engine_destroySprite(&g_assets.playerLivesSprites[i]);
  }
  for (int i = 0; i < PLAYER_STATE_COUNT; i++) {
    engine_destroySprite(&g_assets.playerSprites[i]);
    for (int j = 0; j < DIR_COUNT; j++) {
      engine_destroyAnim(&g_assets.playerAnim[i][j]);
    }
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
  draw_updateGhosts(frameTime, slop);
  draw_updatePlayer(frameTime, slop);
  maze_update(frameTime);
}

void game_draw(void) {
  maze_draw();
  draw_player();
  draw_ghosts();
  draw_interface();
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

int game_getLevel(void) { return g_level; }

void game_over(void) {
  player_totalReset();
  ghost_reset();
  maze_reset();
  g_level = 1;
}

void game_nextLevel(void) {
  player_reset();
  ghost_reset();
  maze_reset();
  g_level += 1;
}

void game_playerDead(void) {
  player_restart();
  ghost_reset();
}
