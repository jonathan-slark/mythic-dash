// clang-format Language: C
#pragma once

#include "engine.h"
#include "log/log.h"

#include <raylib.h>  // Texture2D

// --- Types ---

typedef struct engine_Texture {
  Texture2D handle;
  int width, height;
} engine_Texture;

typedef struct engine_Font {
  Texture2D texture;
  int glyphWidth;
  int glyphHeight;
  int columns;
  int rows;
} engine_Font;

typedef struct engine__ScreenState {
  int width;
  int height;
  int refreshRate;
  int scale;
} engine__ScreenState;

// --- Global state ---

extern engine__ScreenState engine__screenState;
extern log_Log* engine_log;
