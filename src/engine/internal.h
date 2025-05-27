// clang-format Language: C
#pragma once

#include <raylib.h>  // Texture2D
#include "engine.h"
#include "log/log.h"

// --- Types ---

typedef struct engine_Texture {
  Texture2D texture;
} engine_Texture;

typedef struct engine_Font {
  Texture2D texture;
  int       glyphWidth;
  int       glyphHeight;
  int       columns;
  int       rows;
  int       asciiStart;
  int       asciiEnd;
  int       glyphSpacing;
} engine_Font;

typedef struct engine__ScreenState {
  int width;
  int height;
  int refreshRate;
  int scale;
} engine__ScreenState;

// --- Global state ---

extern engine__ScreenState engine__screenState;
extern log_Log*            engine__log;
