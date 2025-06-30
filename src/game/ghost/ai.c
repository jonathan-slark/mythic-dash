#include <assert.h>
#include <limits.h>
#include <math.h>
#include <raylib.h>
#include "../actor/actor.h"
#include "../internal.h"
#include "../maze/maze.h"
#include "ghost.h"
#include "internal.h"

// --- Types ---

typedef struct GhostStateHandler {
  void      (*update)(ghost__Ghost*, float, float);
  game_Tile (*getTarget)(ghost__Ghost*);
  game_Dir  (*selectDir)(ghost__Ghost*, game_Dir*, int);
} GhostStateHandler;

// --- Function Prototypes ---

static game_Dir  selectDirRandom(ghost__Ghost* ghost, game_Dir* dirs, int count);
static game_Dir  selectDirGreedy(ghost__Ghost* ghost, game_Dir* dirs, int count);
static game_Tile getCornerTile(ghost__Ghost* ghost);
static game_Tile getTargetTile(ghost__Ghost* ghost);
static game_Tile getStartTile(ghost__Ghost* ghost);

// --- Constants ---

static const GhostStateHandler FrightenedHandler = { .update = ghost__frightened, .selectDir = selectDirRandom };

static const GhostStateHandler ChaseHandler = {
  .update    = ghost__chase,
  .getTarget = getTargetTile,
  .selectDir = selectDirGreedy
};

static const GhostStateHandler ScatterHandler = {
  .update    = ghost__scatter,
  .getTarget = getCornerTile,
  .selectDir = selectDirGreedy
};

static const GhostStateHandler DeadHandler = {
  .update    = ghost__dead,
  .getTarget = getStartTile,
  .selectDir = selectDirGreedy
};

// --- Helper functions ---

static inline game_Tile getCornerTile(ghost__Ghost* ghost) { return ghost->cornerTile; }

static inline game_Tile getStartTile(ghost__Ghost* ghost) { return maze_getTile(ghost->mazeStart); }

static int getValidDirs(game__Actor* actor, game_Dir currentDir, game_Dir* validDirs, bool isChangedState, float slop) {
  assert(actor != nullptr);
  assert(currentDir >= 0 && currentDir < DIR_COUNT);
  assert(validDirs != nullptr);
  assert(slop > 0.0f);

  game_Dir opposite = game_getOppositeDir(currentDir);
  int      count    = 0;

  for (game_Dir dir = DIR_UP; dir < DIR_COUNT; dir++) {
    if ((isChangedState || dir != opposite) && actor_canMove(actor, dir, slop)) {
      validDirs[count++] = dir;
    }
  }
  return count;
}

static inline game_Dir randomSelect(game_Dir dirs[], int count) {
  assert(count > 0);
  return dirs[GetRandomValue(0, count - 1)];
}

static inline game_Dir selectDirRandom(ghost__Ghost* ghost [[maybe_unused]], game_Dir* dirs, int count) {
  return randomSelect(dirs, count);
}

static game_Dir greedyDirSelect(ghost__Ghost* ghost, game_Dir dirs[], int count, game_Tile targetTile) {
  assert(count > 0);

  if (count == 1) return dirs[0];

  game_Dir bestDirs[DIR_COUNT];
  int      bestDirCount = 0;
  int      minDist      = INT_MAX;
  for (int i = 0; i < count; i++) {
    game_Tile nextTile = actor_nextTile(ghost->actor, dirs[i]);
    int       dist     = maze_manhattanDistance(nextTile, targetTile);
    LOG_TRACE(game_log, "Ghost %d: direction = %s, dist = %d", ghost->id, DIR_STRINGS[dirs[i]], dist);
    if (dist < minDist) {
      bestDirCount = 0;
    }
    if (dist <= minDist) {
      bestDirs[bestDirCount++] = dirs[i];
      minDist                  = dist;
    }
  }
  assert(bestDirCount > 0);
  if (bestDirCount == 1) {
    LOG_TRACE(game_log, "Ghost %d going: %s (best choice)", ghost->id, DIR_STRINGS[bestDirs[0]]);
    return bestDirs[0];
  } else {
    game_Dir dir = randomSelect(bestDirs, bestDirCount);
    LOG_TRACE(game_log, "Ghost %d going: %s (%d choices)", ghost->id, DIR_STRINGS[bestDirs[0]], bestDirCount);
    return dir;
  }
}

static inline game_Dir selectDirGreedy(ghost__Ghost* ghost, game_Dir* dirs, int count) {
  return greedyDirSelect(ghost, dirs, count, ghost->targetTile);
}

// Ghost personalities
static game_Tile getTargetTile(ghost__Ghost* ghost) {
  game_Tile targetTile;
  game_Tile playerTile = maze_getTile(actor_getCentre(player_getActor()));
  switch (ghost->id) {
    // TODO: Change ghost id's to match this order
    // Directly target player's current tile
    case 1: targetTile = playerTile; break;
    // Target four tiles ahead of player, based on his current direction
    case 2: targetTile = player_tileAhead(4); break;
    // Uses a vector based on both Ghost1's position and four tiles ahead of player
    case 0:
      game_Tile ghost1Tile = maze_getTile(actor_getCentre(ghost_getActor(1)));
      targetTile           = maze_doubleVectorBetween(ghost1Tile, player_tileAhead(2));
      break;
    // Chases player until close, then retreats to corner
    case 3:
      if (maze_manhattanDistance(maze_getTile(actor_getCentre(ghost->actor)), playerTile) < 8) {
        targetTile = ghost->cornerTile;
      } else {
        targetTile = playerTile;
      }
      break;
    default: assert(false);
  }
  return targetTile;
}

static void ghostUpdateCommon(ghost__Ghost* ghost, float frameTime, float slop, const GhostStateHandler* handler) {
  game__Actor* actor      = ghost->actor;
  game_Dir     currentDir = actor_getDir(actor);

  actor_move(actor, currentDir, frameTime);
  if (ghost->decisionCooldown > 0.0f) ghost->decisionCooldown = fmaxf(ghost->decisionCooldown - frameTime, 0.0f);

  if (ghost->isChangedState || !actor_canMove(actor, currentDir, slop) || ghost->decisionCooldown == 0.0f) {
    game_Dir validDirs[DIR_COUNT - 1];
    int      count        = getValidDirs(actor, currentDir, validDirs, ghost->isChangedState, slop);
    ghost->isChangedState = false;

    if (count == 0) {
      game_Tile tile = maze_getTile(actor_getPos(actor));
      LOG_ERROR(game_log, "Ghost %u has no valid directions at (%d, %d)", ghost->id, tile.col, tile.row);
      actor_setDir(actor, game_getOppositeDir(currentDir));
      ghost->decisionCooldown = DECISION_COOLDOWN;
    } else {
      if (handler->getTarget) ghost->targetTile = handler->getTarget(ghost);

      game_Dir newDir = handler->selectDir(ghost, validDirs, count);

      if (count > 1 || currentDir != newDir) {
        actor_setDir(actor, newDir);
        LOG_TRACE(game_log, "Ghost %u chose new direction: %s", ghost->id, DIR_STRINGS[newDir]);
        ghost->decisionCooldown = DECISION_COOLDOWN;
      }
    }
  }
}

// --- Ghost state functions ---

// Ghost moves back and forth in pen till released
void ghost__pen(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor      = ghost->actor;
  game_Dir     currentDir = actor_getDir(actor);
  actor_move(actor, currentDir, frameTime);
  if (!actor_canMove(actor, currentDir, slop)) actor_setDir(actor, game_getOppositeDir(currentDir));

  // Release the ho... er... ghosts!
  if (ghost->startTimer <= frameTime) {
    ghost->startTimer = 0.0f;
    ghost->update     = ghost__penToStart;
  } else {
    ghost->startTimer -= frameTime;
  }
}

// Ghost moves from pen to start position
void ghost__penToStart(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor = ghost->actor;
  assert(actor != nullptr);
  game_Dir dir = actor_getDir(actor);

  float   startY = ghost->mazeStart.y;
  Vector2 pos    = actor_getPos(actor);
  if (fabsf(pos.y - startY) > slop) {
    LOG_TRACE(game_log, "Moving to line up wth exit");
    dir = pos.y < startY ? DIR_DOWN : DIR_UP;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
  } else {
    LOG_TRACE(game_log, "Moving to start tile");
    float startX = ghost->mazeStart.x;
    dir          = pos.x < startX ? DIR_RIGHT : DIR_LEFT;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
    if (fabsf(pos.x - startX) < slop) {
      actor_setPos(actor, (Vector2) { startX, pos.y });
      actor_setDir(actor, GHOST_START_DIR);
      actor_setSpeed(actor, ghost__getSpeed());
      ghost->update = g_state.update;
    }
  }
}

// Ghost moves from maze start back to pen
void ghost__startToPen(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor = ghost->actor;
  assert(actor != nullptr);
  game_Dir dir = actor_getDir(actor);

  float   startY = MAZE_CENTRE.y;
  Vector2 pos    = actor_getPos(actor);
  if (fabsf(pos.y - startY) > slop) {
    LOG_TRACE(game_log, "Moving to line up wth exit");
    dir = pos.y < startY ? DIR_DOWN : DIR_UP;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
  } else {
    LOG_TRACE(game_log, "Moving to centre tile");
    float startX = MAZE_CENTRE.x;
    dir          = pos.x < startX ? DIR_RIGHT : DIR_LEFT;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
    if (fabsf(pos.x - startX) < slop) {
      actor_setPos(actor, (Vector2) { startX, pos.y });
      actor_setSpeed(ghost->actor, SPEED_SLOW);
      ghost->update = ghost__penToStart;
    }
  }
}

// Ghost wanders randomly
void ghost__frightened(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  ghostUpdateCommon(ghost, frameTime, slop, &FrightenedHandler);
}

// Ghost chases player
void ghost__chase(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  ghostUpdateCommon(ghost, frameTime, slop, &ChaseHandler);
}

// Ghost heads to it's assigned corner
void ghost__scatter(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  ghostUpdateCommon(ghost, frameTime, slop, &ScatterHandler);
}

// Ghost heads back to the pen
void ghost__dead(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  ghostUpdateCommon(ghost, frameTime, slop, &DeadHandler);

  Vector2   pos       = actor_getPos(ghost->actor);
  game_Tile startTile = maze_getTile(ghost->mazeStart);
  Vector2   dest      = { startTile.col * TILE_SIZE, startTile.row * TILE_SIZE };
  if (fabsf(pos.x - dest.x) < slop && fabsf(pos.y - dest.y) < slop) {
    ghost->update = ghost__startToPen;
  }
}
