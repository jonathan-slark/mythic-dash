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

static const float FRAME_TIME = (1.0f / 12.0f);

static const Vector2 PLAYER_OFFSET = {0.0f, 0.0f};
static const AnimData PLAYER_ANIMS[DIR_COUNT] = {
    [DIR_UP] = {0, 0, 3, FRAME_TIME},
    [DIR_RIGHT] = {1, 0, 3, FRAME_TIME},
    [DIR_DOWN] = {2, 0, 3, FRAME_TIME},
    [DIR_LEFT] = {3, 0, 3, FRAME_TIME}};

static const Vector2 GHOST_OFFSETS[GHOST_COUNT] = {
    {48.0f, 0.0f}, {80.0f, 0.0f}, {112.0f, 0.0f}, {144.0f, 0.0f}};

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
