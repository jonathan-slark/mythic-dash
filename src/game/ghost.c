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

static const float SPEEDS[]          = { 25.0f, 40.0f, 50.f, 100.0f };
static const float DECISION_COOLDOWN = 0.5f;

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

static inline game__Dir getOppositeDir(game__Dir dir) { return (dir + 2) % DIR_COUNT; }

static int getValidDirs(game__Actor* actor, game__Dir current_dir, game__Dir* validDirs, float slop) {
  game__Dir opposite = getOppositeDir(current_dir);
  int       count    = 0;

  for (game__Dir dir = DIR_UP; dir < DIR_COUNT; dir++) {
    if (dir != opposite && actor_canMove(actor, dir, slop)) {
      validDirs[count++] = dir;
    }
  }
  return count;
}

static inline game__Dir randomSelect(game__Dir* directions, int count) {
  return directions[GetRandomValue(0, count - 1)];
}

static void idle(Ghost* ghost [[maybe_unused]], float frameTime [[maybe_unused]], float slop [[maybe_unused]]) {}

static void wander(Ghost* ghost, float frameTime, float slop) {
  game__Dir currentDir = actor_getDir(ghost->actor);
  actor_move(ghost->actor, currentDir, frameTime);

  ghost->decisionCooldown -= frameTime;
  if (ghost->decisionCooldown < 0.0f) ghost->decisionCooldown = 0.0f;

  // Check if we hit a wall or can make a direction decision
  if (!actor_canMove(ghost->actor, currentDir, slop) || ghost->decisionCooldown == 0.0f) {
    game__Dir validDirs[DIR_COUNT - 1];
    int       count = getValidDirs(ghost->actor, currentDir, validDirs, slop);

    if (count == 0) {
      // Should never happen - log error
      Vector2 pos = actor_getPos(ghost->actor);
      LOG_ERROR(game__log, "Ghost %u has no valid directions at (%.2f, %.2f)", ghost->id, pos.x, pos.y);
      actor_setDir(ghost->actor, getOppositeDir(currentDir));
      ghost->decisionCooldown = DECISION_COOLDOWN;
    } else {
      game__Dir newDir = randomSelect(validDirs, count);
      // Check if we were at a junction
      if (count > 1 || currentDir != newDir) {
        actor_setDir(ghost->actor, newDir);
        LOG_DEBUG(game__log, "Ghost %u chose new direction: %s", ghost->id, DIR_STRINGS[newDir]);
        ghost->decisionCooldown = DECISION_COOLDOWN;
      }
    }
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
