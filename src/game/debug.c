#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "actor.h"
#include "internal.h"

// --- Types ---

typedef struct Debug {
  bool isFPSOverlayEnabled;
  bool isMazeOverlayEnabled;
  bool isPlayerOverlayEnabled;
  bool isGhostOverlayEnabled;
} Debug;

// --- Constants ---

constexpr int BUFFER_SIZE = 32;
#define OVERLAY_COLOUR_PLAYER (Color){ 100, 200, 255, 128 }
#define OVERLAY_COLOUR_GHOST (Color){ 255, 128, 200, 128 }
#define OVERLAY_NUMBER_SIZE 20
#define OVERLAY_TEXT_SIZE 12

// --- Global state ---

static Debug g_debug = {
  .isFPSOverlayEnabled    = false,
  .isMazeOverlayEnabled   = false,
  .isPlayerOverlayEnabled = false,
  .isGhostOverlayEnabled  = false
};

// --- Helper functions ---

static void drawNumber(float number, Vector2 pos, int size, Color colour) {
  assert(size > 0);

  char numberText[BUFFER_SIZE];

  snprintf(numberText, sizeof(numberText), "%.2f", number);
  DrawText(numberText, pos.x, pos.y, size, colour);
}

static void drawArrow(Vector2 start, Vector2 end, float size, Color colour) {
  assert(size > 0);

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
  assert(actor != nullptr);

  int     scale = engine_getScale();
  Vector2 pos   = Vector2Scale(POS_ADJUST(actor_getPos(actor)), scale);
  Vector2 start = Vector2Add(pos, (Vector2) { ACTOR_SIZE * scale / 2.0f, ACTOR_SIZE * scale / 2.0f });

  Vector2 end;
  switch (actor_getDir(actor)) {
    case DIR_UP: end = Vector2Add(pos, (Vector2) { ACTOR_SIZE * scale / 2.0f, 0.0f }); break;
    case DIR_RIGHT: end = Vector2Add(pos, (Vector2) { ACTOR_SIZE * scale, ACTOR_SIZE * scale / 2.0f }); break;
    case DIR_DOWN: end = Vector2Add(pos, (Vector2) { ACTOR_SIZE * scale / 2.0f, ACTOR_SIZE * scale }); break;
    case DIR_LEFT: end = Vector2Add(pos, (Vector2) { 0.0f, ACTOR_SIZE * scale / 2.0f }); break;
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

  if (g_debug.isMazeOverlayEnabled) {
    DrawText("Maze overlay enabled", 0, yPos, 20, BLUE);
    yPos += 20;
    maze_tilesOverlay();
  }

  if (g_debug.isPlayerOverlayEnabled) {
    DrawText("Player overlay enabled", 0, yPos, 20, ORANGE);
    yPos += 20;
    actor_overlay(player_getActor(), OVERLAY_COLOUR_PLAYER);
    actor_canMoveOverlay(player_getActor());
    // actor_moveOverlay(player_getActor());
  }

  if (g_debug.isGhostOverlayEnabled) {
    DrawText("Ghost overlay enabled", 0, yPos, 20, OVERLAY_COLOUR_GHOST);
    yPos += 20;

    for (int i = 0; i < GHOST_COUNT; i++) {
      game__Actor* actor = ghost_getActor(i);
      actor_overlay(actor, OVERLAY_COLOUR_GHOST);
      actor_canMoveOverlay(actor);
      drawActorArrow(actor);

      float   cooldown = ghost_getDecisionCooldown(i);
      Color   colour   = cooldown == 0.0f ? WHITE : RED;
      int     scale    = engine_getScale();
      Vector2 pos      = Vector2Scale(POS_ADJUST(actor_getPos(actor)), scale);
      drawNumber(cooldown, Vector2AddValue(pos, 1), OVERLAY_NUMBER_SIZE, BLACK);
      drawNumber(cooldown, pos, OVERLAY_NUMBER_SIZE, colour);

      const char* str = ghost_getStateStr(i);
      DrawText(str, pos.x + 1, pos.y + ACTOR_SIZE * scale - OVERLAY_TEXT_SIZE + 1, OVERLAY_TEXT_SIZE, BLACK);
      DrawText(str, pos.x, pos.y + ACTOR_SIZE * scale - OVERLAY_TEXT_SIZE, OVERLAY_TEXT_SIZE, WHITE);
    }
  }
}

void debug_toggleFPSOverlay(void) { g_debug.isFPSOverlayEnabled = !g_debug.isFPSOverlayEnabled; }

void debug_toggleMazeOverlay(void) { g_debug.isMazeOverlayEnabled = !g_debug.isMazeOverlayEnabled; }

void debug_togglePlayerOverlay(void) { g_debug.isPlayerOverlayEnabled = !g_debug.isPlayerOverlayEnabled; }

void debug_toggleGhostOverlay(void) { g_debug.isGhostOverlayEnabled = !g_debug.isGhostOverlayEnabled; }
