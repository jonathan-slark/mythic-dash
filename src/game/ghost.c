#include <assert.h>
#include <math.h>
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

static void pen(Ghost* ghost, float frameTime, float slop);
static void penToStart(Ghost* ghost, float frameTime, float slop);
static void wander(Ghost* ghost, float frameTime, float slop);

// --- Constants ---

static const float SPEEDS[]          = { 25.0f, 40.0f, 50.f, 100.0f };
static const float DECISION_COOLDOWN = 0.5f;
static const float PEN_TOP           = 108.0f;
static const float PEN_BOT           = 116.0f;

static const Vector2   GHOST_MAZE_START = { 132.0f, 88.0f };
static const game__Dir GHOST_START_DIR  = DIR_LEFT;
static const float     GHOST_CHASETIMER = 10.0f;
static const struct {
  Vector2   startPos;
  game__Dir startDir;
  float     startSpeed;
  float     startTimer;
  void      (*update)(Ghost*, float, float);
} CREATURE_DATA[CREATURE_COUNT] = {
  [0] = {   { 1 * TILE_SIZE, 1 * TILE_SIZE }, DIR_RIGHT, SPEEDS[SpeedNormal], GHOST_CHASETIMER, wander },
  [1] = {  { 28 * TILE_SIZE, 1 * TILE_SIZE },  DIR_LEFT, SPEEDS[SpeedNormal],             0.0f, wander },
  [2] = {  { 1 * TILE_SIZE, 14 * TILE_SIZE }, DIR_RIGHT, SPEEDS[SpeedNormal],            10.0f, wander },
  [3] = { { 28 * TILE_SIZE, 14 * TILE_SIZE },  DIR_LEFT, SPEEDS[SpeedNormal],             20.f, wander },
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

// Ghost moves up and down in pen till released
static void pen(Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor = ghost->actor;
  assert(actor != nullptr);
  game__Dir dir = actor_getDir(actor);
  assert(dir == DIR_UP || dir == DIR_DOWN);

  actor_moveNoCheck(actor, dir, frameTime);
  Vector2 pos = actor_getPos(actor);

  if (pos.y <= PEN_TOP) {
    actor_setDir(actor, DIR_DOWN);
    actor_setPos(actor, (Vector2) { pos.x, PEN_TOP });
  } else if (pos.y >= PEN_BOT) {
    actor_setDir(actor, DIR_UP);
    actor_setPos(actor, (Vector2) { pos.x, PEN_BOT });
  }

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

  float   startX = GHOST_MAZE_START.x;  // First ghost starts outside
  Vector2 pos    = actor_getPos(actor);
  if (fabsf(pos.x - startX) > slop) {
    // Move to below door
    LOG_TRACE(game__log, "Moving to below door");
    dir = pos.x < startX ? DIR_RIGHT : DIR_LEFT;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
  } else {
    // Move through door, we're a ghost!
    LOG_TRACE(game__log, "Moving though door");
    actor_setDir(actor, DIR_UP);
    actor_moveNoCheck(actor, DIR_UP, frameTime);
    float startY = GHOST_MAZE_START.y;
    if (pos.y <= startY) {
      actor_setPos(actor, (Vector2) { pos.x, startY });
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
