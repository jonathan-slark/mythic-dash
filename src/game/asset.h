/*
 * anim.h: Animation data, only to be used by game.c
 */

#include "game.h"
#include <raylib.h>

// --- Types ---

typedef struct AnimData {
  int row;
  int startCol;
  int frameCount;
  float frameTime;
} AnimData;

typedef struct ActorData {
  Vector2 size;
  Vector2 offset;
  Vector2 inset;
  bool loop;
  AnimData animData[DIR_COUNT];
} ActorData;

typedef struct game__Assets {
  engine_Texture *creatureSpriteSheet;
  engine_Texture *playerSpriteSheet;
  engine_Sprite *playerSprites[PLAYER_STATE_COUNT];
  engine_Sprite *playerLivesSprites[PLAYER_LIVES];
  engine_Anim *playerAnim[PLAYER_STATE_COUNT][DIR_COUNT];
  engine_Sprite *creatureSprites[CREATURE_COUNT];
  engine_Anim *creatureAnims[CREATURE_COUNT][DIR_COUNT];
  engine_Font *font;
} game__Assets;

// --- Constants ---

static const char FILE_BACKGROUND[] = ASSET_DIR "gfx/background.png";
static const char FILE_CREATURES[] = ASSET_DIR "gfx/creatures.png";
static const char FILE_PLAYER[] = ASSET_DIR "gfx/player.png";
static const char FILE_FONT[] = ASSET_DIR "gfx/font.png";

static const float FRAME_TIME = 0.1f;

static const Vector2 PLAYER_LIVES_OFFSET = {8.0f, 248.0f};

// clang-format off
static const ActorData PLAYER_DATA[PLAYER_STATE_COUNT] = {
  {
    .inset    = { 8.0f, 8.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 4, .startCol = 0, .frameCount = 5, FRAME_TIME},
      [DIR_RIGHT] = { .row = 2, .startCol = 0, .frameCount = 5, FRAME_TIME},
      [DIR_DOWN]  = { .row = 0, .startCol = 0, .frameCount = 5, FRAME_TIME},
      [DIR_LEFT]  = { .row = 6, .startCol = 0, .frameCount = 5, FRAME_TIME}
    }
  },
  {
    .inset    = { 8.0f, 8.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 4, .startCol = 5, .frameCount = 4, FRAME_TIME},
      [DIR_RIGHT] = { .row = 2, .startCol = 5, .frameCount = 4, FRAME_TIME},
      [DIR_DOWN]  = { .row = 0, .startCol = 5, .frameCount = 4, FRAME_TIME},
      [DIR_LEFT]  = { .row = 6, .startCol = 5, .frameCount = 4, FRAME_TIME}
    }
  },
  {
    .inset    = { 8.0f, 8.0f },
    .loop = false,
    .animData = {
      [DIR_UP]    = { .row = 4, .startCol = 21, .frameCount = 3, FRAME_TIME},
      [DIR_RIGHT] = { .row = 2, .startCol = 21, .frameCount = 3, FRAME_TIME},
      [DIR_DOWN]  = { .row = 0, .startCol = 21, .frameCount = 3, FRAME_TIME},
      [DIR_LEFT]  = { .row = 6, .startCol = 21, .frameCount = 3, FRAME_TIME}
    }
  }
};

static const ActorData CREATURE_DATA[CREATURE_COUNT] = {
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f  },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 0, .frameCount = 3, FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 0, .frameCount = 3, FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 0, .frameCount = 3, FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 0, .frameCount = 3, FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 3, .frameCount = 3, FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 3, .frameCount = 3, FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 3, .frameCount = 3, FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 3, .frameCount = 3, FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 6, .frameCount = 3, FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 6, .frameCount = 3, FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 6, .frameCount = 3, FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 6, .frameCount = 3, FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 9, .frameCount = 3, FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 9, .frameCount = 3, FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 9, .frameCount = 3, FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 9, .frameCount = 3, FRAME_TIME }
    }
  }
};
// clang-format on

// --- Global state ---

extern game__Assets g_assets;
extern int g_level;
