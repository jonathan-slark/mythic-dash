#include "debug.h"
#include <assert.h>
#include <engine/engine.h>
#include <raylib.h>
#include <raymath.h>
#include "../actor/actor.h"
#include "../actor/internal.h"
#include "../ghost/ghost.h"
#include "../internal.h"
#include "../maze/maze.h"
#include "../player/player.h"

// --- Types ---

typedef struct Debug {
  bool isFPSOverlayEnabled;
  bool isMazeOverlayEnabled;
  bool isPlayerOverlayEnabled;
  bool isGhostOverlayEnabled;
  bool isPlayerImmune;
} Debug;

// --- Constants ---

#define OVERLAY_COLOUR_PLAYER (Color){ 100, 200, 255, 128 }
#define OVERLAY_COLOUR_GHOST (Color){ 255, 128, 200, 128 }
#define OVERLAY_NUMBER_SIZE 20
#define OVERLAY_LARGE_TEXT_SIZE 20
#define OVERLAY_TEXT_SIZE 12
static const Vector2 OVERLAY_FPS_POS      = { 600.0f, 0.0f };
static const Vector2 OVERLAY_IMMUNE_POS   = { 400.0f, 0.0f };
static const Vector2 OVERLAY_STATE_NUM    = { 210.0f, 0.0f };
static const Vector2 OVERLAY_STATE_STRING = { 220.0f, 0.0f };
static const Vector2 OVERLAY_STATE_TIMER  = { 260.0f, 0.0f };

// --- Global state ---

static Debug g_debug = {
  .isFPSOverlayEnabled    = false,
  .isMazeOverlayEnabled   = false,
  .isPlayerOverlayEnabled = false,
  .isGhostOverlayEnabled  = false,
  .isPlayerImmune         = false
};

// --- Helper functions ---

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

  engine_drawArrow(start, end, ACTOR_SIZE, WHITE);
}

// --- Debug functions ---

void debug_drawOverlay(void) {
  if (g_debug.isPlayerImmune) {
    DrawText("Player immune", OVERLAY_IMMUNE_POS.x, OVERLAY_IMMUNE_POS.y, OVERLAY_LARGE_TEXT_SIZE, RED);
  }

  if (g_debug.isFPSOverlayEnabled) {
    DrawFPS(OVERLAY_FPS_POS.x, OVERLAY_FPS_POS.y);
  }

  if (g_debug.isMazeOverlayEnabled) {
    maze_tilesOverlay();
  }

  if (g_debug.isPlayerOverlayEnabled) {
    actor_overlay(player_getActor(), OVERLAY_COLOUR_PLAYER);
    actor_canMoveOverlay(player_getActor());
    engine_drawFloat(actor_getSpeed(player_getActor()), POS_ADJUST(player_getPos()), OVERLAY_NUMBER_SIZE, WHITE);
  }

  if (g_debug.isGhostOverlayEnabled) {
    for (int i = 0; i < CREATURE_COUNT; i++) {
      game__Actor* actor = ghost_getActor(i);
      actor_overlay(actor, OVERLAY_COLOUR_GHOST);
      actor_canMoveOverlay(actor);
      drawActorArrow(actor);

      float   cooldown = ghost_getDecisionCooldown(i);
      Color   colour   = cooldown == 0.0f ? WHITE : RED;
      Vector2 pos      = POS_ADJUST(actor_getPos(actor));
      engine_drawFloat(cooldown, pos, OVERLAY_NUMBER_SIZE, colour);

      const char* string = ghost_getStateString(i);
      float       scale  = engine_getScale();
      engine_drawText(
          string, (Vector2) { pos.x, pos.y + ACTOR_SIZE - OVERLAY_TEXT_SIZE / scale }, OVERLAY_TEXT_SIZE, WHITE
      );

      pos.y += 4;
      engine_drawFloat(actor_getSpeed(actor), pos, OVERLAY_NUMBER_SIZE, WHITE);

      engine_drawInt(ghost_getGlobaStateNum() - 1, OVERLAY_STATE_NUM, OVERLAY_NUMBER_SIZE, WHITE);
      engine_drawText(ghost_getGlobalStateString(), OVERLAY_STATE_STRING, OVERLAY_LARGE_TEXT_SIZE, WHITE);
      engine_drawFloat(ghost_getGlobalTimer(), OVERLAY_STATE_TIMER, OVERLAY_NUMBER_SIZE, WHITE);

      Vector2 start        = POS_ADJUST(actor_getPos(actor));
      start                = Vector2AddValue(start, TILE_SIZE / 2.0f);
      game_Tile targetTile = ghost_getTarget(i);
      if (targetTile.col >= 0 && targetTile.row >= 0) {
        Vector2 end = POS_ADJUST(maze_getPos(targetTile));
        end         = Vector2AddValue(end, TILE_SIZE / 2.0f);
        engine_drawLine(start, end, BLACK);
      }
    }
  }
}

void debug_toggleFPSOverlay(void) { g_debug.isFPSOverlayEnabled = !g_debug.isFPSOverlayEnabled; }

void debug_toggleMazeOverlay(void) { g_debug.isMazeOverlayEnabled = !g_debug.isMazeOverlayEnabled; }

void debug_togglePlayerOverlay(void) { g_debug.isPlayerOverlayEnabled = !g_debug.isPlayerOverlayEnabled; }

void debug_toggleGhostOverlay(void) { g_debug.isGhostOverlayEnabled = !g_debug.isGhostOverlayEnabled; }

void debug_togglePlayerImmune(void) { g_debug.isPlayerImmune = !g_debug.isPlayerImmune; }

bool debug_isPlayerImmune(void) { return g_debug.isPlayerImmune; }
