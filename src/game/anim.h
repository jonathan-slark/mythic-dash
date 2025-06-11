/*
 * anim.h: Animation data, only to be used by game.c
 */

#include "internal.h"
#include <raylib.h>

// --- Types ---

typedef struct AnimData {
  int row;
  int startCol;
  int frameCount;
  float frameTime;
} AnimData;

typedef struct ActorData {
  Vector2 offset;
  AnimData animData[DIR_COUNT];
} ActorData;

static const float FRAME_TIME = (1.0f / 12.0f);

// clang-format off
static const ActorData PLAYER_DATA = {
  .offset   = { 0.0f, 0.0f },
  .animData = {
    [DIR_UP]    = { .row = 0, .startCol = 0, .frameCount = 3, FRAME_TIME},
    [DIR_RIGHT] = { .row = 1, .startCol = 0, .frameCount = 3, FRAME_TIME},
    [DIR_DOWN]  = { .row = 2, .startCol = 0, .frameCount = 3, FRAME_TIME},
    [DIR_LEFT]  = { .row = 3, .startCol = 0, .frameCount = 3, FRAME_TIME}
  }
};

static const ActorData GHOST_DATA[GHOST_COUNT] = {
  {
    .offset   = { 48.0f, 0.0f },
    .animData = {
      [DIR_UP]    = { .row = 0, .startCol = 3, .frameCount = 2, FRAME_TIME },
      [DIR_RIGHT] = { .row = 1, .startCol = 3, .frameCount = 2, FRAME_TIME },
      [DIR_DOWN]  = { .row = 2, .startCol = 3, .frameCount = 2, FRAME_TIME },
      [DIR_LEFT]  = { .row = 3, .startCol = 3, .frameCount = 2, FRAME_TIME }
    }
  },
  {
    .offset   = { 80.0f, 0.0f },
    .animData = {
      [DIR_UP]    = { .row = 0, .startCol = 5, .frameCount = 2, FRAME_TIME },
      [DIR_RIGHT] = { .row = 1, .startCol = 5, .frameCount = 2, FRAME_TIME },
      [DIR_DOWN]  = { .row = 2, .startCol = 5, .frameCount = 2, FRAME_TIME },
      [DIR_LEFT]  = { .row = 3, .startCol = 5, .frameCount = 2, FRAME_TIME }
    },
  },
  {
    .offset   = { 112.0f, 0.0f },
    .animData = {
      [DIR_UP]    = { .row = 0, .startCol = 7, .frameCount = 2, FRAME_TIME },
      [DIR_RIGHT] = { .row = 1, .startCol = 7, .frameCount = 2, FRAME_TIME },
      [DIR_DOWN]  = { .row = 2, .startCol = 7, .frameCount = 2, FRAME_TIME },
      [DIR_LEFT]  = { .row = 3, .startCol = 7, .frameCount = 2, FRAME_TIME }
    },
  },
  {
    .offset   = { 144.0f, 0.0f },
    .animData = {
      [DIR_UP]    = { .row = 0, .startCol = 9, .frameCount = 2, FRAME_TIME },
      [DIR_RIGHT] = { .row = 1, .startCol = 9, .frameCount = 2, FRAME_TIME },
      [DIR_DOWN]  = { .row = 2, .startCol = 9, .frameCount = 2, FRAME_TIME },
      [DIR_LEFT]  = { .row = 3, .startCol = 9, .frameCount = 2, FRAME_TIME }
    }
  }
};
// clang-format on
