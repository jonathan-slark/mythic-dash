#include <assert.h>
#include "internal.h"

// --- Types ---

typedef enum GhostSpeed { SpeedSlow, SpeedNormal, SpeedFast } GhostSpeed;

typedef struct Ghost {
  void         (*update)(struct Ghost*, float, float);
  float        timer;
  game__Actor* actor;

#ifndef NDEBUG
  unsigned id;
#endif  // NDEBUG

} Ghost;

// --- Function prototypes ---

static void wander(Ghost* ghost, float frameTime, float slop);

// --- Constants ---

static const float SPEEDS[] = { 25.0f, 40.0f, 50.f, 100.0f };

static const struct {
  Vector2   startPos;
  game__Dir startDir;
  float     startSpeed;
  void      (*update)(Ghost*, float, float);
} GHOST_DATA[GHOST_COUNT] = {
  [0] = {  { 108.0f, 88.0f }, DIR_LEFT, SPEEDS[SpeedSlow], wander },
  [1] = { { 108.0f, 112.0f },   DIR_UP, SPEEDS[SpeedSlow], wander },
  [2] = {  { 92.0f, 112.0f }, DIR_DOWN, SPEEDS[SpeedSlow], wander },
  [3] = { { 124.0f, 112.0f }, DIR_DOWN, SPEEDS[SpeedSlow], wander },
};

// --- Global state ---

static Ghost g_ghosts[GHOST_COUNT];

// --- Helper functions ---

static void wander(Ghost* ghost, float frameTime, float slop [[maybe_unused]]) {
  if (!actor_isMoving(ghost->actor)) {
    LOG_DEBUG(game__log, "Ghost (%d) not moving", ghost->id);
  }

  actor_move(ghost->actor, actor_getDir(ghost->actor), frameTime);
}

// --- Ghost functions ---

bool ghost_init(void) {
  for (int i = 0; i < GHOST_COUNT; i++) {
    assert(g_ghosts[i].actor == nullptr);
    g_ghosts[i].update = GHOST_DATA[i].update;
    g_ghosts[i].timer  = 0.0f;
    g_ghosts[i].actor  = actor_create(
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
