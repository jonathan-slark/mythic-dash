#include "internal.h"

// --- Types ---

typedef struct Debug {
  bool isFPSOverlayEnabled;
  bool isMoveOverlayEnabled;
  bool isCanMoveOverlayEnabled;
} Debug;

// --- Global state ---

static Debug g_debug = {.isFPSOverlayEnabled = false, .isMoveOverlayEnabled = false, .isCanMoveOverlayEnabled = false};

void         debug_drawOverlay(void) {
  if (g_debug.isFPSOverlayEnabled) {
    DrawFPS(0, 0);
  }
  if (g_debug.isMoveOverlayEnabled) {
    player_overlay();
    actor_moveOverlay(player_getActor());
  }
  if (g_debug.isCanMoveOverlayEnabled) {
    player_overlay();
    actor_canMoveOverlay(player_getActor());
  }
}

void debug_toggleFPSOverlay(void) { g_debug.isFPSOverlayEnabled = !g_debug.isFPSOverlayEnabled; }

void debug_toggleMoveOverlay(void) { g_debug.isMoveOverlayEnabled = !g_debug.isMoveOverlayEnabled; }

void debug_toggleCanMoveOverlay(void) { g_debug.isCanMoveOverlayEnabled = !g_debug.isCanMoveOverlayEnabled; }
