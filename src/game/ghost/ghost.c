#include "ghost.h"
#include <assert.h>
#include "../game.h"
#include "log/log.h"

// --- Constants ---

static const float STATE_TIMERS[]       = { 7.0f, 20.0f, 7.0f, 20.0f, 5.0f, 20.0f, 5.0f };
static const char* STATE_PEN_STR        = "PEN";
static const char* STATE_PETTOSTART_STR = "PEN2STA";
static const char* STATE_FRIGHTENED_STR = "FRIGHT";
static const char* STATE_CHASE_STR      = "CHASE";
static const char* STATE_SCATTER_STR    = "SCATTER";

// --- Global state ---

ghost__State g_state = { .update = nullptr, .stateNum = 0, .stateTimer = 0.0f };

// --- Helper functions ---

// Update the global state and change ghost states
static void updateState(float frameTime) {
  g_state.stateTimer -= frameTime;
  if (g_state.stateTimer < 0) g_state.stateTimer = 0.0f;

  if (g_state.stateTimer == 0.0f) {
    if (g_state.stateNum == COUNT(STATE_TIMERS)) {
      // Permament chase
      for (int i = 0; i < CREATURE_COUNT; i++) {
        g_state.ghosts[i].update         = ghost__chase;
        g_state.ghosts[i].isChangedState = true;
      }
      g_state.stateNum++;
    } else if (g_state.stateNum < COUNT(STATE_TIMERS)) {
      // Toggle between scatter and chase
      if (g_state.update == nullptr || g_state.update == ghost__chase) {
        g_state.update = ghost__scatter;
      } else {
        g_state.update = ghost__chase;
      }

      for (int i = 0; i < CREATURE_COUNT; i++) {
        if (g_state.ghosts[i].update != ghost__pen && g_state.ghosts[i].update != ghost__penToStart &&
            g_state.ghosts[i].update != ghost__frightened) {
          g_state.ghosts[i].update         = g_state.update;
          g_state.ghosts[i].isChangedState = true;
        }
      }

      g_state.stateTimer = STATE_TIMERS[g_state.stateNum++];
      LOG_INFO(game__log, "Changing to state: %s", ghost_getStateString(1));
    }
  }
}

// --- Ghost functions ---

bool ghost_init(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_state.ghosts[i].actor == nullptr);
    g_state.ghosts[i].update     = CREATURE_DATA[i].update;
    g_state.ghosts[i].timer      = CREATURE_DATA[i].startTimer;
    g_state.ghosts[i].mazeStart  = CREATURE_DATA[i].mazeStart;
    g_state.ghosts[i].targetTile = (game__Tile) { -1, -1 };
    g_state.ghosts[i].cornerTile = CREATURE_DATA[i].cornerTile;
    LOG_INFO(game__log, "Corner tile %d, %d", g_state.ghosts[i].cornerTile.col, g_state.ghosts[i].cornerTile.row);
    g_state.ghosts[i].decisionCooldown = 0.0f;
    g_state.ghosts[i].actor            = actor_create(
        CREATURE_DATA[i].startPos,
        (Vector2) { ACTOR_SIZE, ACTOR_SIZE },
        CREATURE_DATA[i].startDir,
        CREATURE_DATA[i].startSpeed
    );
    if (g_state.ghosts[i].actor == nullptr) return false;
#ifndef NDEBUG
    g_state.ghosts[i].id = i;
#endif
  }
  return true;
}

void ghost_shutdown(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_state.ghosts[i].actor != nullptr);
    actor_destroy(&g_state.ghosts[i].actor);
    assert(g_state.ghosts[i].actor == nullptr);
  }
}

void ghost_update(float frameTime, float slop) {
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_state.ghosts[i].actor != nullptr);
    g_state.ghosts[i].update(&g_state.ghosts[i], frameTime, slop);
  }
  updateState(frameTime);
}

Vector2 ghost_getPos(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.ghosts[id].actor != nullptr);
  return actor_getPos(g_state.ghosts[id].actor);
}

game__Dir ghost_getDir(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.ghosts[id].actor != nullptr);
  return actor_getDir(g_state.ghosts[id].actor);
}

game__Actor* ghost_getActor(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.ghosts[id].actor != nullptr);
  return g_state.ghosts[id].actor;
}

float ghost_getDecisionCooldown(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.ghosts[id].actor != nullptr);
  return g_state.ghosts[id].decisionCooldown;
}

game__Tile ghost_getTarget(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  return g_state.ghosts[id].targetTile;
}

float ghost_getGlobalTimer(void) { return g_state.stateTimer; }
int   ghost_getGlobaStateNum(void) { return g_state.stateNum; }

const char* ghost_getStateString(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.ghosts[id].actor != nullptr);

  if (g_state.ghosts[id].update == ghost__pen) {
    return STATE_PEN_STR;
  }
  if (g_state.ghosts[id].update == ghost__penToStart) {
    return STATE_PETTOSTART_STR;
  }
  if (g_state.ghosts[id].update == ghost__frightened) {
    return STATE_FRIGHTENED_STR;
  }
  if (g_state.ghosts[id].update == ghost__chase) {
    return STATE_CHASE_STR;
  }
  if (g_state.ghosts[id].update == ghost__scatter) {
    return STATE_SCATTER_STR;
  }

  assert(false);
}
