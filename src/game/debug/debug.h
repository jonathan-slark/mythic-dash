// clang-format Language: C
#pragma once

#include <engine/engine.h>
#include "../input/input.h"
#include "../internal.h"

// --- Helper functions ---

#ifndef NDEBUG
static const float FPS[] = { 15, 30, 60, 0 };
#endif

static inline void checkFPSKeys(void) {
#ifdef NDEBUG
  (void) 0;
#else
  if (input_isKeyPressed(INPUT_MINUS)) {
    g_game.fpsIndex = (g_game.fpsIndex == 0) ? COUNT(FPS) - 1 : g_game.fpsIndex - 1;
  }
  if (input_isKeyPressed(INPUT_EQUAL)) {
    g_game.fpsIndex = (g_game.fpsIndex == COUNT(FPS) - 1) ? 0 : g_game.fpsIndex + 1;
  }
  // SetTargetFPS(FPS[g_game.fpsIndex]);
#endif
}

// --- Debug functions ---

void debug_reset(void);
void debug_drawOverlay(void);
void debug_toggleFPSOverlay(void);
void debug_toggleMazeOverlay(void);
void debug_togglePlayerOverlay(void);
void debug_toggleCreatureOverlay(void);
void debug_togglePlayerImmune(void);
bool debug_isPlayerImmune(void);
