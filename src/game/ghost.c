#include <assert.h>
#include <math.h>
#include "game.h"

// --- Types ---

typedef enum GhostSpeed { SpeedSlow, SpeedNormal, SpeedFast } GhostSpeed;

typedef struct Ghost {
  void         (*update)(struct Ghost*, float, float);
  float        timer;
  Vector2      mazeStart;
  float        decisionCooldown;
  game__Actor* actor;

#ifndef NDEBUG
  unsigned id;
#endif  // NDEBUG

} Ghost;

// --- Function prototypes ---

static void pen(Ghost* ghost, float frameTime, float slop);
static void penToStart(Ghost* ghost, float frameTime, float slop);
static void wander(Ghost* ghost, float frameTime, float slop);

// --- Constants ---

static const float SPEEDS[]          = { 25.0f, 40.0f, 50.f, 100.0f };
static const float DECISION_COOLDOWN = 0.5f;

static const Vector2 GHOST_MAZE_START[] = {
  { 11 * TILE_SIZE, 7 * TILE_SIZE },
  { 17 * TILE_SIZE, 7 * TILE_SIZE }
};
static const game__Dir GHOST_START_DIR  = DIR_LEFT;
static const float     GHOST_CHASETIMER = 10.0f;
static const struct {
  Vector2   startPos;
  Vector2   mazeStart;
  game__Dir startDir;
  float     startSpeed;
  float     startTimer;
  void      (*update)(Ghost*, float, float);
} CREATURE_DATA[CREATURE_COUNT] = {
  [0] = { { 13 * TILE_SIZE, 6 * TILE_SIZE },
         GHOST_MAZE_START[0],
         DIR_DOWN, SPEEDS[SpeedSlow],
         GHOST_CHASETIMER * 0.0f,
         pen },
  [1] = { { 15 * TILE_SIZE, 6 * TILE_SIZE },
         GHOST_MAZE_START[1],
         DIR_LEFT, SPEEDS[SpeedSlow],
         GHOST_CHASETIMER * 1.0f,
         pen },
  [2] = { { 13 * TILE_SIZE, 9 * TILE_SIZE },
         GHOST_MAZE_START[0],
         DIR_RIGHT, SPEEDS[SpeedSlow],
         GHOST_CHASETIMER * 2.0f,
         pen },
  [3] = { { 15 * TILE_SIZE, 9 * TILE_SIZE },
         GHOST_MAZE_START[1],
         DIR_UP, SPEEDS[SpeedSlow],
         GHOST_CHASETIMER * 3.0f,
         pen },
};

static const char* STATE_PEN_STR        = "PEN";
static const char* STATE_PETTOSTART_STR = "PEN2STA";
static const char* STATE_WANDER_STR     = "WANDER";

// --- Global state ---

static Ghost g_ghosts[CREATURE_COUNT];

// --- Helper functions ---

static inline game__Dir getOppositeDir(game__Dir dir) {
  assert(dir >= 0 && dir < DIR_COUNT);
  return (dir + 2) % DIR_COUNT;
}

static inline game__Dir randomSelect(game__Dir* dirs, int count) {
  assert(dirs != nullptr);
  assert(count > 0);
  return dirs[GetRandomValue(0, count - 1)];
}

static int getValidDirs(game__Actor* actor, game__Dir currentDir, game__Dir* validDirs, float slop) {
  assert(actor != nullptr);
  assert(currentDir >= 0 && currentDir < DIR_COUNT);
  assert(validDirs != nullptr);
  assert(slop > 0.0f);

  game__Dir opposite = getOppositeDir(currentDir);
  int       count    = 0;

  for (game__Dir dir = DIR_UP; dir < DIR_COUNT; dir++) {
    if (dir != opposite && actor_canMove(actor, dir, slop)) {
      validDirs[count++] = dir;
    }
  }
  return count;
}

// Ghost moves back and forth in pen till released
static void pen(Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor      = ghost->actor;
  game__Dir    currentDir = actor_getDir(actor);
  actor_move(actor, currentDir, frameTime);
  if (!actor_canMove(actor, currentDir, slop)) actor_setDir(actor, getOppositeDir(currentDir));

  // Release the ho... er... ghosts!
  if (ghost->timer <= frameTime) {
    ghost->timer  = 0.0f;
    ghost->update = penToStart;
  } else {
    ghost->timer -= frameTime;
  }
}

// Ghost moves from pen to start position
static void penToStart(Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor = ghost->actor;
  assert(actor != nullptr);
  game__Dir dir = actor_getDir(actor);

  float   startY = ghost->mazeStart.y;
  Vector2 pos    = actor_getPos(actor);
  if (fabsf(pos.y - startY) > slop) {
    LOG_TRACE(game__log, "Moving to line up wth exit");
    dir = pos.y < startY ? DIR_DOWN : DIR_UP;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
  } else {
    LOG_TRACE(game__log, "Moving to start tile");
    float startX = ghost->mazeStart.x;
    dir          = pos.x < startX ? DIR_RIGHT : DIR_LEFT;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
    if (fabsf(pos.x - startX) < slop) {
      actor_setPos(actor, (Vector2) { startX, pos.y });
      actor_setDir(actor, GHOST_START_DIR);
      actor_setSpeed(actor, SPEEDS[SpeedNormal]);
      ghost->timer  = GHOST_CHASETIMER;
      ghost->update = wander;
    }
  }
}

// Ghost wanders randomly
static void wander(Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor      = ghost->actor;
  game__Dir    currentDir = actor_getDir(actor);
  actor_move(actor, currentDir, frameTime);

  ghost->decisionCooldown -= frameTime;
  if (ghost->decisionCooldown < 0.0f) ghost->decisionCooldown = 0.0f;

  // Check if we hit a wall or can make a direction decision
  if (!actor_canMove(actor, currentDir, slop) || ghost->decisionCooldown == 0.0f) {
    game__Dir validDirs[DIR_COUNT - 1];
    int       count = getValidDirs(actor, currentDir, validDirs, slop);

    if (count == 0) {
      // Should never happen - log error
      Vector2 pos = actor_getPos(actor);
      LOG_ERROR(game__log, "Ghost %u has no valid directions at (%.2f, %.2f)", ghost->id, pos.x, pos.y);
      actor_setDir(actor, getOppositeDir(currentDir));
      ghost->decisionCooldown = DECISION_COOLDOWN;
    } else {
      game__Dir newDir = randomSelect(validDirs, count);
      // Check if we were at a junction
      if (count > 1 || currentDir != newDir) {
        actor_setDir(actor, newDir);
        LOG_DEBUG(game__log, "Ghost %u chose new direction: %s", ghost->id, DIR_STRINGS[newDir]);
        ghost->decisionCooldown = DECISION_COOLDOWN;
      }
    }
  }
}

// --- Ghost functions ---

bool ghost_init(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    assert(g_ghosts[i].actor == nullptr);
    g_ghosts[i].update           = CREATURE_DATA[i].update;
    g_ghosts[i].timer            = CREATURE_DATA[i].startTimer;
    g_ghosts[i].mazeStart        = CREATURE_DATA[i].mazeStart;
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

const char* ghost_getStateString(int id) {
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(id >= 0 && id < CREATURE_COUNT);
  assert(g_ghosts[id].actor != nullptr);

  if (g_ghosts[id].update == pen) {
    return STATE_PEN_STR;
  }
  if (g_ghosts[id].update == penToStart) {
    return STATE_PETTOSTART_STR;
  }
  if (g_ghosts[id].update == wander) {
    return STATE_WANDER_STR;
  }

  assert(false);
}
