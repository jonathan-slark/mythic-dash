#include "ghost.h"
#include <assert.h>
#include "../game.h"
#include "ghost.h"
#include "log/log.h"

// --- Constants ---

static const float STATE_TIMERS[]       = { 7.0f, 20.0f, 7.0f, 20.0f, 5.0f, 20.0f, 5.0f };
static const char* STATE_PEN_STR        = "PEN";
static const char* STATE_PETTOSTART_STR = "PEN2STA";
static const char* STATE_STARTTOPEN_STR = "STA2PEN";
static const char* STATE_FRIGHTENED_STR = "FRIGHT";
static const char* STATE_DEAD_STR       = "DEAD";
static const char* STATE_CHASE_STR      = "CHASE";
static const char* STATE_SCATTER_STR    = "SCATTER";

// --- Global state ---

ghost__State g_state = { .update = nullptr, .lastUpdate = nullptr, .stateNum = 0, .stateTimer = 0.0f };

// --- Helper functions ---

static inline void updateTimer(float frameTime) { g_state.stateTimer = fmaxf(g_state.stateTimer - frameTime, 0.0f); }
static inline bool isPermanentChaseState(void) { return g_state.stateNum == COUNT(STATE_TIMERS); }

static inline bool shouldTransitionState(void) {
  return g_state.stateTimer == 0.0f && g_state.stateNum <= COUNT(STATE_TIMERS);
}

static inline bool shouldUpdateGhostState(ghost__Ghost* ghost) {
  return ghost->update != ghost__pen && ghost->update != ghost__penToStart && ghost->update != ghost__dead &&
         ghost->update != ghost__startToPen;
}

static const char* getStateString(void (*update)(struct ghost__Ghost*, float, float)) {
  if (update == ghost__pen) return STATE_PEN_STR;
  if (update == ghost__penToStart) return STATE_PETTOSTART_STR;
  if (update == ghost__startToPen) return STATE_STARTTOPEN_STR;
  if (update == ghost__frightened) return STATE_FRIGHTENED_STR;
  if (update == ghost__dead) return STATE_DEAD_STR;
  if (update == ghost__chase) return STATE_CHASE_STR;
  if (update == ghost__scatter) return STATE_SCATTER_STR;
  return "";
}

static void transitionToState(void (*newState)(ghost__Ghost*, float, float)) {
  g_state.lastUpdate = g_state.update;
  g_state.update     = newState;
  for (int i = 0; i < CREATURE_COUNT; i++) {
    if (shouldUpdateGhostState(&g_state.ghosts[i])) {
      g_state.ghosts[i].update         = newState;
      g_state.ghosts[i].isChangedState = true;
    }
  }
}

static void transitionToPermanentChase(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    g_state.ghosts[i].update         = ghost__chase;
    g_state.ghosts[i].isChangedState = true;
  }
  g_state.stateNum++;
}

static void toggleGhostState() {
  void (*newState)(
      ghost__Ghost*, float, float
  ) = (g_state.update == nullptr || g_state.update == ghost__chase) ? ghost__scatter : ghost__chase;
  transitionToState(newState);
  g_state.stateTimer = STATE_TIMERS[g_state.stateNum++];
  LOG_TRACE(game__log, "Changing to state: %s", getStateString(newState));
}

// Update the global state and change ghost states
static void updateState(float frameTime) {
  updateTimer(frameTime);

  if (shouldTransitionState()) {
    if (isPermanentChaseState()) {
      transitionToPermanentChase();
    } else {
      toggleGhostState();
    }
  }
}

static void ghostResetTargets(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    g_state.ghosts[i].targetTile = DEFAULT_TARGET_TILE;
  }
}

static void ghostDefaults(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    g_state.ghosts[i].update           = CREATURE_DATA[i].update;
    g_state.ghosts[i].timer            = CREATURE_DATA[i].startTimer;
    g_state.ghosts[i].mazeStart        = CREATURE_DATA[i].mazeStart;
    g_state.ghosts[i].cornerTile       = CREATURE_DATA[i].cornerTile;
    g_state.ghosts[i].decisionCooldown = 0.0f;
  }
  ghostResetTargets();
}

static void ghostSetSpeeds(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    if (shouldUpdateGhostState(&g_state.ghosts[i])) actor_setSpeed(g_state.ghosts[i].actor, ghost__getSpeed());
  }
}

// --- Ghost functions ---

bool ghost_init(void) {
  ghostDefaults();
  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_state.ghosts[i].actor == nullptr);
    g_state.ghosts[i].actor = actor_create(
        CREATURE_DATA[i].startPos,
        (Vector2) { ACTOR_SIZE, ACTOR_SIZE },
        CREATURE_DATA[i].startDir,
        CREATURE_DATA[i].startSpeed
    );
    if (g_state.ghosts[i].actor == nullptr) return false;
    g_state.ghosts[i].id = i;
  }
  return true;
}

void ghost_reset(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    ghostDefaults();
    actor_setPos(g_state.ghosts[i].actor, CREATURE_DATA[i].startPos);
    actor_setDir(g_state.ghosts[i].actor, CREATURE_DATA[i].startDir);
    actor_setSpeed(g_state.ghosts[i].actor, CREATURE_DATA[i].startSpeed);
  }
  g_state.update     = nullptr;
  g_state.stateNum   = 0;
  g_state.stateTimer = 0.0f;
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

  bool              playerDead  = false;
  game__PlayerState playerState = player_getState();
  game__AABB        playerAABB  = actor_getAABB(player_getActor());

  for (int i = 0; i < CREATURE_COUNT; i++) {
    g_state.ghosts[i].update(&g_state.ghosts[i], frameTime, slop);

    game__AABB ghostAABB = actor_getAABB(g_state.ghosts[i].actor);
    if (aabb_isColliding(playerAABB, ghostAABB)) {
      if (playerState == PLAYER_SWORD) {
        g_state.ghosts[i].update = ghost__dead;
      } else if (g_state.ghosts[i].update != ghost__dead) {
        playerDead = true;
      }
    }
  }

  if (!player_hasSword()) updateState(frameTime);

  if (playerDead && playerState != PLAYER_DEAD) player_dead();
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

float ghost_getGlobalTimer(void) { return player_hasSword() ? player_getSwordTimer() : g_state.stateTimer; }

int ghost_getGlobaStateNum(void) { return g_state.stateNum; }

const char* ghost_getGlobalStateString(void) { return getStateString(g_state.update); }

const char* ghost_getStateString(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.ghosts[id].actor != nullptr);

  return getStateString(g_state.ghosts[id].update);
}

void ghost_swordPickup(void) {
  transitionToState(ghost__frightened);
  ghostSetSpeeds();
  ghostResetTargets();
}

void ghost_swordDrop(void) {
  transitionToState(g_state.lastUpdate);
  ghostSetSpeeds();
}

bool ghost_isFrightened(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.ghosts[id].update != nullptr);
  return g_state.ghosts[id].update == ghost__frightened;
}

bool ghost_isDead(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_state.ghosts[id].update != nullptr);
  return g_state.ghosts[id].update == ghost__dead || g_state.ghosts[id].update == ghost__startToPen;
}
