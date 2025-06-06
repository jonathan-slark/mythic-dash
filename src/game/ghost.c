#include <assert.h>
#include "internal.h"

// --- Types ---

typedef enum GhostSpeed { SpeedSlow, SpeedNormal, SpeedFast } GhostSpeed;

typedef struct Ghost {
  void         (*update)(struct Ghost*, float, float);
  float        timer;
  float        decisionCooldown;
  game__Actor* actor;

#ifndef NDEBUG
  unsigned id;
#endif  // NDEBUG

} Ghost;

// --- Function prototypes ---

static void idle(Ghost* ghost, float frameTime, float slop);
static void wander(Ghost* ghost, float frameTime, float slop);

// --- Constants ---

static const float     SPEEDS[]                = { 25.0f, 40.0f, 50.f, 100.0f };
static const game__Dir OPPOSITE_DIR[DIR_COUNT] = { DIR_DOWN, DIR_LEFT, DIR_UP, DIR_RIGHT };
static const float     DECISION_COOLDOWN       = 1.0f / 3.0f;

static const struct {
  Vector2   startPos;
  game__Dir startDir;
  float     startSpeed;
  void      (*update)(Ghost*, float, float);
} GHOST_DATA[GHOST_COUNT] = {
  [0] = {  { 108.0f, 88.0f }, DIR_LEFT, SPEEDS[SpeedSlow], wander },
  [1] = { { 108.0f, 112.0f },   DIR_UP, SPEEDS[SpeedSlow],   idle },
  [2] = {  { 92.0f, 112.0f }, DIR_DOWN, SPEEDS[SpeedSlow],   idle },
  [3] = { { 124.0f, 112.0f }, DIR_DOWN, SPEEDS[SpeedSlow],   idle },
};

// --- Global state ---

static Ghost g_ghosts[GHOST_COUNT];

// --- Helper functions ---

static void      idle(Ghost* ghost [[maybe_unused]], float frameTime [[maybe_unused]], float slop [[maybe_unused]]) {}

static game__Dir randomDir(Ghost* ghost, float slop) {
  // Record available directions
  game__Dir currentDir = actor_getDir(ghost->actor);
  game__Dir canMove[DIR_COUNT];
  int       canMoveCount = 0;
  for (game__Dir dir = 0; dir < DIR_COUNT; dir++) {
    if (dir == OPPOSITE_DIR[currentDir]) continue;  // Don't go back the way we came
    if (actor_canMove(ghost->actor, dir, slop)) canMove[canMoveCount++] = dir;
  }

  if (canMoveCount == 0) assert(false);
  if (canMoveCount == 1) {
    return canMove[0];
  } else {
    return canMove[GetRandomValue(0, canMoveCount - 1)];
  }
}

static void wander(Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(ghost->decisionCooldown >= 0.0f);
  assert(frameTime >= 0.0f);
  assert(slop >= 0.0f);

  game__Dir dir = actor_getDir(ghost->actor);
  actor_move(ghost->actor, dir, frameTime);

  if (actor_isMoving(ghost->actor)) {
    if (ghost->decisionCooldown == 0.0f) {
      // Try changing direction
      game__Dir newDir = randomDir(ghost, slop);
      if (dir != newDir) {
        actor_setDir(ghost->actor, newDir);
        ghost->decisionCooldown = DECISION_COOLDOWN;
        LOG_DEBUG(game__log, "Made decision: dir is %s", DIR_STRINGS[dir]);
      }
    } else {
      ghost->decisionCooldown -= frameTime;
      if (ghost->decisionCooldown < 0.0f) ghost->decisionCooldown = 0.0f;
    }
  } else {
    // If ghost stopped change direction to keep up the flow
    actor_move(ghost->actor, randomDir(ghost, slop), frameTime);
    ghost->decisionCooldown = DECISION_COOLDOWN;
    LOG_DEBUG(game__log, "Made decision: dir is %s", DIR_STRINGS[actor_getDir(ghost->actor)]);
  }
}

// --- Ghost functions ---

bool ghost_init(void) {
  for (int i = 0; i < GHOST_COUNT; i++) {
    assert(g_ghosts[i].actor == nullptr);
    g_ghosts[i].update           = GHOST_DATA[i].update;
    g_ghosts[i].timer            = 0.0f;
    g_ghosts[i].decisionCooldown = 0.0f;
    g_ghosts[i].actor            = actor_create(
        GHOST_DATA[i].startPos, (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, GHOST_DATA[i].startDir, GHOST_DATA[i].startSpeed
    );
    if (g_ghosts[i].actor == nullptr) return false;
#ifndef NDEBUG
    g_ghosts[i].id = i;
#endif
  }
  return true;
}

void ghost_shutdown(void) {
  for (int i = 0; i < GHOST_COUNT; i++) {
    assert(g_ghosts[i].actor != nullptr);
    actor_destroy(&g_ghosts[i].actor);
    assert(g_ghosts[i].actor == nullptr);
  }
}

void ghost_update(float frameTime, float slop) {
  for (int i = 0; i < GHOST_COUNT; i++) {
    assert(g_ghosts[i].actor != nullptr);
    g_ghosts[i].update(&g_ghosts[i], frameTime, slop);
  }
}

Vector2 ghost_getPos(int id) {
  assert(id >= 0 && id < GHOST_COUNT);
  assert(g_ghosts[id].actor != nullptr);
  return actor_getPos(g_ghosts[id].actor);
}

game__Dir ghost_getDir(int id) {
  assert(id >= 0 && id < GHOST_COUNT);
  assert(g_ghosts[id].actor != nullptr);
  return actor_getDir(g_ghosts[id].actor);
}

game__Actor* ghost_getActor(int id) {
  assert(id >= 0 && id < GHOST_COUNT);
  assert(g_ghosts[id].actor != nullptr);
  return g_ghosts[id].actor;
}

float ghost_getDecisionCooldown(int id) {
  assert(id >= 0 && id < GHOST_COUNT);
  assert(g_ghosts[id].actor != nullptr);
  return g_ghosts[id].decisionCooldown;
}
