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
  engine_Sprite*  playerSprites[PLAYER_STATE_COUNT];
  engine_Sprite*  playerLivesSprites[PLAYER_LIVES];
  engine_Anim*    playerAnim[PLAYER_STATE_COUNT][DIR_COUNT];
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

const char* PLAYER_STATE_STRINGS[PLAYER_STATE_COUNT] = { "NORMAL", "SWORD" };

static const Color GHOST_DEAD_COLOUR = { 255, 255, 255, 100 };

// --- Global state ---

log_Log*            game__log;
static game__Assets g_assets;
static int          g_level = 1;
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
              PLAYER_DATA[i].inset
          )
      );
    }
  }

  Vector2 offset = PLAYER_LIVES_OFFSET;
  for (int i = 0; i < PLAYER_LIVES; i++) {
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
              CREATURE_DATA[i].inset
          )
      );
    }
  }
  return true;
}

static void updatePlayer(float frameTime, float slop) {
  player_update(frameTime, slop);

  static game__PlayerState prevState = PLAYER_NORMAL;
  game__PlayerState        state     = player_getState();
  Vector2                  pos       = POS_ADJUST(player_getPos());
  static game__Dir         prevDir   = DIR_NONE;
  game__Dir                dir       = player_getDir();

  if (state != prevState || dir != prevDir) {
    if (state != prevState)
      LOG_DEBUG(
          game__log, "Player state changed from %s to %s", PLAYER_STATE_STRINGS[prevState], PLAYER_STATE_STRINGS[state]
      );
    if (dir != prevDir)
      LOG_DEBUG(game__log, "Player direction changed from %s to %s", DIR_STRINGS[prevDir], DIR_STRINGS[dir]);
    engine_resetAnim(g_assets.playerAnim[state][dir]);
    prevState = state;
    prevDir   = dir;
  }

  engine_spriteSetPos(g_assets.playerSprites[state], pos);

  if (player_isMoving() || state == PLAYER_SWORD) {
    engine_updateAnim(g_assets.playerAnim[state][dir], frameTime);
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

static void drawGhosts(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    Color colour;
    if (ghost_isFrightened(i))
      colour = BLUE;
    else if (ghost_isDead(i))
      colour = GHOST_DEAD_COLOUR;
    else
      colour = WHITE;
    engine_drawSpriteColoured(g_assets.creatureSpriteSheet, g_assets.creatureSprites[i], colour);
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
  engine_drawSprite(g_assets.playerSpriteSheet, g_assets.playerSprites[player_getState()]);
  for (int i = 0; i < player_getLives() - 1; i++) {
    engine_drawSprite(g_assets.playerSpriteSheet, g_assets.playerLivesSprites[i]);
  }
  drawGhosts();
  engine_fontPrintf(g_assets.font, 8, 0, "SCORE: %d", player_getScore());
  engine_fontPrintf(g_assets.font, 372, 0, "LEVEL: %d / ?", g_level);
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
}

void game_nextLevel(void) {
  player_reset();
  ghost_reset();
  maze_reset();
  g_level += 1;
}
