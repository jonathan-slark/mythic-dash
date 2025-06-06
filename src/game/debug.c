#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "internal.h"

// --- Types ---

typedef struct Debug {
  bool isFPSOverlayEnabled;
  bool isMoveOverlayEnabled;
  bool isCanMoveOverlayEnabled;
  bool isGhostOverlayEnabled;
} Debug;

// --- Constants ---

constexpr int BUFFER_SIZE = 32;

// --- Global state ---

static Debug g_debug = {
  .isFPSOverlayEnabled     = false,
  .isMoveOverlayEnabled    = false,
  .isCanMoveOverlayEnabled = false,
  .isGhostOverlayEnabled   = false
};

// --- Helper functions ---

static void drawNumber(float number, Vector2 pos, int size, Color colour) {
  char numberText[BUFFER_SIZE];

  snprintf(numberText, sizeof(numberText), "%.2f", number);
  DrawText(numberText, pos.x, pos.y, size, colour);
}

static void drawArrow(Vector2 start, Vector2 end, float size, Color colour) {
  DrawLineV(start, end, colour);

  // Calculate angle of the line
  float angle = atan2f(end.y - start.y, end.x - start.x);

  // Arrowhead points
  Vector2 right = { end.x - size * cosf(angle - PI / 6), end.y - size * sinf(angle - PI / 6) };

  Vector2 left  = { end.x - size * cosf(angle + PI / 6), end.y - size * sinf(angle + PI / 6) };

  DrawLineV(end, right, colour);
  DrawLineV(end, left, colour);
}

static void drawActorArrow(game__Actor* actor) {
  Vector2 pos   = Vector2Scale(POS_ADJUST(actor_getPos(actor)), 4.0f);
  Vector2 start = Vector2Add(pos, (Vector2) { ACTOR_SIZE * 2.0f, ACTOR_SIZE * 2.0f });
  Vector2 end;
  switch (actor_getDir(actor)) {
    case DIR_UP: end = Vector2Add(pos, (Vector2) { ACTOR_SIZE * 2.0f, 0.0f }); break;
    case DIR_RIGHT: end = Vector2Add(pos, (Vector2) { ACTOR_SIZE * 4.0f, ACTOR_SIZE * 2.0f }); break;
    case DIR_DOWN: end = Vector2Add(pos, (Vector2) { ACTOR_SIZE * 2.0f, ACTOR_SIZE * 4.0f }); break;
    case DIR_LEFT: end = Vector2Add(pos, (Vector2) { 0.0f, ACTOR_SIZE * 2.0f }); break;
    default: assert(false);
  }
  drawArrow(start, end, ACTOR_SIZE, WHITE);
}

// --- Debug functions ---

void debug_drawOverlay(void) {
  int yPos = 0;

  if (g_debug.isFPSOverlayEnabled) {
    DrawFPS(0, yPos);
    yPos += 20;
  }

  if (g_debug.isMoveOverlayEnabled || g_debug.isCanMoveOverlayEnabled) {
    actor_overlay(player_getActor(), OVERLAY_COLOUR_PLAYER);
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

  if (g_debug.isGhostOverlayEnabled) {
    DrawText("Ghost overlay enabled", 0, yPos, 20, OVERLAY_COLOUR_GHOST);
    yPos += 20;
    for (int i = 0; i < GHOST_COUNT; i++) {
      game__Actor* actor = ghost_getActor(i);
      actor_overlay(actor, OVERLAY_COLOUR_GHOST);
      float cooldown = ghost_getDecisionCooldown(i);
      Color colour   = cooldown == 0.0f ? WHITE : RED;
      drawNumber(cooldown, Vector2Scale(POS_ADJUST(actor_getPos(actor)), 4.0f), OVERLAY_TEXT_SIZE, colour);
      drawActorArrow(actor);
    }
  }
}

void debug_toggleFPSOverlay(void) { g_debug.isFPSOverlayEnabled = !g_debug.isFPSOverlayEnabled; }

void debug_toggleMoveOverlay(void) { g_debug.isMoveOverlayEnabled = !g_debug.isMoveOverlayEnabled; }

void debug_toggleCanMoveOverlay(void) { g_debug.isCanMoveOverlayEnabled = !g_debug.isCanMoveOverlayEnabled; }

void debug_toggleGhostOverlay(void) { g_debug.isGhostOverlayEnabled = !g_debug.isGhostOverlayEnabled; }
