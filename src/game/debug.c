#include "internal.h"

// --- Types ---

typedef struct Debug {
  bool isFPSOverlayEnabled;
  bool isMoveOverlayEnabled;
  bool isCanMoveOverlayEnabled;
} Debug;

// --- Global state ---

static Debug g_debug = { .isFPSOverlayEnabled     = false,
                         .isMoveOverlayEnabled    = false,
                         .isCanMoveOverlayEnabled = false };

void         debug_drawOverlay(void) {
  int yPos = 0;

  if (g_debug.isFPSOverlayEnabled) {
    DrawFPS(0, yPos);
    yPos += 20;
  }

  if (g_debug.isMoveOverlayEnabled || g_debug.isCanMoveOverlayEnabled) {
    player_overlay();
  }

  if (g_debug.isMoveOverlayEnabled) {
    DrawText("Move overlay enabled", 0, yPos, 20, ORANGE);
    yPos += 20;
    actor_moveOverlay(player_getActor());
  }

  if (g_debug.isCanMoveOverlayEnabled) {
    DrawText("Can move overlay enabled", 0, yPos, 20, YELLOW);
    yPos += 20;
    actor_canMoveOverlay(player_getActor());
  }
}

void debug_toggleFPSOverlay(void) { g_debug.isFPSOverlayEnabled = !g_debug.isFPSOverlayEnabled; }

void debug_toggleMoveOverlay(void) { g_debug.isMoveOverlayEnabled = !g_debug.isMoveOverlayEnabled; }

void debug_toggleCanMoveOverlay(void) { g_debug.isCanMoveOverlayEnabled = !g_debug.isCanMoveOverlayEnabled; }
