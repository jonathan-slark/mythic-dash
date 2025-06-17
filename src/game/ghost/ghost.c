#include "ghost.h"
#include <assert.h>
#include "../game.h"

// --- Global state ---

ghost__Ghost g_ghosts[CREATURE_COUNT];

// --- ghost__Ghost functions ---

bool ghost_init(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_ghosts[i].actor == nullptr);
    g_ghosts[i].update           = CREATURE_DATA[i].update;
    g_ghosts[i].timer            = CREATURE_DATA[i].startTimer;
    g_ghosts[i].mazeStart        = CREATURE_DATA[i].mazeStart;
    g_ghosts[i].cornerTile       = CREATURE_DATA[i].cornerTile;
    g_ghosts[i].decisionCooldown = 0.0f;
    g_ghosts[i].actor            = actor_create(
        CREATURE_DATA[i].startPos,
        (Vector2) { ACTOR_SIZE, ACTOR_SIZE },
        CREATURE_DATA[i].startDir,
        CREATURE_DATA[i].startSpeed
    );
    if (g_ghosts[i].actor == nullptr) return false;
#ifndef NDEBUG
    g_ghosts[i].id = i;
#endif
  }
  return true;
}

void ghost_shutdown(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_ghosts[i].actor != nullptr);
    actor_destroy(&g_ghosts[i].actor);
    assert(g_ghosts[i].actor == nullptr);
  }
}

void ghost_update(float frameTime, float slop) {
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_ghosts[i].actor != nullptr);
    g_ghosts[i].update(&g_ghosts[i], frameTime, slop);
  }
}

Vector2 ghost_getPos(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_ghosts[id].actor != nullptr);
  return actor_getPos(g_ghosts[id].actor);
}

game__Dir ghost_getDir(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_ghosts[id].actor != nullptr);
  return actor_getDir(g_ghosts[id].actor);
}

game__Actor* ghost_getActor(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_ghosts[id].actor != nullptr);
  return g_ghosts[id].actor;
}

float ghost_getDecisionCooldown(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_ghosts[id].actor != nullptr);
  return g_ghosts[id].decisionCooldown;
}

game__Tile ghost_getTarget(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  return g_ghosts[id].targetTile;
}
