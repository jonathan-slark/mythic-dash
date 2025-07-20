#include <assert.h>
#include <limits.h>
#include <math.h>
#include <raylib.h>
#include "../actor/actor.h"
#include "../audio/audio.h"
#include "../internal.h"
#include "../maze/maze.h"
#include "../player/player.h"
#include "creature.h"
#include "internal.h"
#include "log/log.h"

// --- Types ---

typedef struct CreatureStateHandler {
  void      (*update)(creature_Creature*, float, float);
  game_Tile (*getTarget)(creature_Creature*);
  game_Dir  (*selectDir)(creature_Creature*, game_Dir*, int);
} CreatureStateHandler;

// --- Function Prototypes ---

static game_Dir  selectDirRandom(creature_Creature* creature, game_Dir* dirs, int count);
static game_Dir  selectDirGreedy(creature_Creature* creature, game_Dir* dirs, int count);
static game_Tile getCornerTile(creature_Creature* creature);
static game_Tile getTargetTile(creature_Creature* creature);
static game_Tile getStartTile(creature_Creature* creature);

// --- Constants ---

static const CreatureStateHandler FrightenedHandler = { .update = creature_frightened, .selectDir = selectDirRandom };

static const CreatureStateHandler ChaseHandler = {
  .update    = creature_chase,
  .getTarget = getTargetTile,
  .selectDir = selectDirGreedy
};

static const CreatureStateHandler ScatterHandler = {
  .update    = creature_scatter,
  .getTarget = getCornerTile,
  .selectDir = selectDirGreedy
};

static const CreatureStateHandler DeadHandler = {
  .update    = creature_dead,
  .getTarget = getStartTile,
  .selectDir = selectDirGreedy
};

// --- Helper functions ---

static inline game_Tile getCornerTile(creature_Creature* creature) { return creature->cornerTile; }

static inline game_Tile getStartTile(creature_Creature* creature) { return maze_getTile(creature->mazeStart); }

static int getValidDirs(game_Actor* actor, game_Dir currentDir, game_Dir* validDirs, bool isChangedState, float slop) {
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

static inline game_Dir selectDirRandom(creature_Creature* creature [[maybe_unused]], game_Dir* dirs, int count) {
  return randomSelect(dirs, count);
}

static game_Dir greedyDirSelect(creature_Creature* creature, game_Dir dirs[], int count, game_Tile targetTile) {
  assert(count > 0);

  if (count == 1) return dirs[0];

  game_Dir bestDirs[DIR_COUNT];
  int      bestDirCount = 0;
  int      minDist      = INT_MAX;
  for (int i = 0; i < count; i++) {
    game_Tile nextTile = actor_nextTile(creature->actor, dirs[i]);
    int       dist     = maze_manhattanDistance(nextTile, targetTile);
    if (dist < minDist) {
      bestDirCount = 0;
    }
    if (dist <= minDist) {
      bestDirs[bestDirCount++] = dirs[i];
      minDist                  = dist;
    }
    LOG_TRACE(
        game_log,
        "Creature %d: direction = %s, dist = %d, bestDirCount = %d",
        creature->id,
        DIR_STRINGS[dirs[i]],
        dist,
        bestDirCount
    );
  }

  assert(bestDirCount > 0);
  if (bestDirCount == 1) {
    LOG_TRACE(game_log, "Creature %d going: %s (best choice)", creature->id, DIR_STRINGS[bestDirs[0]]);
    return bestDirs[0];
  } else {
    // Keep Arcade Mode deterministic
    game_Dir dir = DIR_NONE;
    if (game_getDifficulty() == DIFFICULTY_ARCADE) {
      dir = bestDirs[0];
    } else {
      dir = randomSelect(bestDirs, bestDirCount);
    }
    LOG_TRACE(game_log, "Creature %d going: %s (%d choices)", creature->id, DIR_STRINGS[dir], bestDirCount);
    return dir;
  }
}

static inline game_Dir selectDirGreedy(creature_Creature* creature, game_Dir* dirs, int count) {
  return greedyDirSelect(creature, dirs, count, creature->targetTile);
}

// Creature personalities
static game_Tile getTargetTile(creature_Creature* creature) {
  game_Tile targetTile;
  game_Tile playerTile = maze_getTile(actor_getCentre(player_getActor()));
  switch (creature->id) {
    // TODO: Change creature id's to match this order
    // Directly target player's current tile
    case 1: targetTile = playerTile; break;
    // Target four tiles ahead of player, based on his current direction
    case 2: targetTile = player_tileAhead(4); break;
    // Uses a vector based on both Creature1's position and four tiles ahead of player
    case 0:
      game_Tile creature1Tile = maze_getTile(actor_getCentre(creature_getActor(1)));
      targetTile              = maze_doubleVectorBetween(creature1Tile, player_tileAhead(2));
      break;
    // Chases player until close, then retreats to corner
    case 3:
      if (maze_manhattanDistance(maze_getTile(actor_getCentre(creature->actor)), playerTile) < 8) {
        targetTile = creature->cornerTile;
      } else {
        targetTile = playerTile;
      }
      break;
    default: assert(false);
  }
  return targetTile;
}

static void
creatureUpdateCommon(creature_Creature* creature, float frameTime, float slop, const CreatureStateHandler* handler) {
  game_Actor* actor      = creature->actor;
  game_Dir    currentDir = actor_getDir(actor);

  actor_move(actor, currentDir, frameTime);
  if (creature->decisionCooldown > 0.0f)
    creature->decisionCooldown = fmaxf(creature->decisionCooldown - frameTime, 0.0f);

  if (creature->isChangedState || !actor_canMove(actor, currentDir, slop) || creature->decisionCooldown == 0.0f) {
    game_Dir validDirs[DIR_COUNT - 1];
    int      count           = getValidDirs(actor, currentDir, validDirs, creature->isChangedState, slop);
    creature->isChangedState = false;

    if (count == 0) {
      game_Tile tile = maze_getTile(actor_getPos(actor));
      LOG_ERROR(game_log, "Creature %u has no valid directions at (%d, %d)", creature->id, tile.col, tile.row);
      actor_setDir(actor, game_getOppositeDir(currentDir));
      creature->decisionCooldown = DECISION_COOLDOWN;
    } else {
      if (handler->getTarget) creature->targetTile = handler->getTarget(creature);

      game_Dir newDir = handler->selectDir(creature, validDirs, count);

      if (count > 1 || currentDir != newDir) {
        actor_setDir(actor, newDir);
        LOG_TRACE(game_log, "Creature %u chose new direction: %s", creature->id, DIR_STRINGS[newDir]);
        creature->decisionCooldown = DECISION_COOLDOWN;
      }
    }
  }
}

// --- Creature state functions ---

// Creature moves back and forth in pen till released
void creature_pen(creature_Creature* creature, float frameTime, float slop) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game_Actor* actor      = creature->actor;
  game_Dir    currentDir = actor_getDir(actor);
  actor_move(actor, currentDir, frameTime);
  if (!actor_canMove(actor, currentDir, slop)) actor_setDir(actor, game_getOppositeDir(currentDir));

  // Release the ho... er... creatures!
  if (creature->startTimer <= frameTime) {
    if (!player_hasSword()) {
      creature->startTimer = 0.0f;
      creature->update     = creature_penToStart;
    }
  } else {
    creature->startTimer -= frameTime;
  }
}

// Creature moves from pen to start position
void creature_penToStart(creature_Creature* creature, float frameTime, float slop) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game_Actor* actor = creature->actor;
  assert(actor != nullptr);
  game_Dir dir = actor_getDir(actor);

  float   startY = creature->mazeStart.y;
  Vector2 pos    = actor_getPos(actor);
  if (fabsf(pos.y - startY) > slop) {
    LOG_TRACE(game_log, "Moving to line up wth exit");
    dir = pos.y < startY ? DIR_DOWN : DIR_UP;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
  } else {
    LOG_TRACE(game_log, "Moving to start tile");
    float startX = creature->mazeStart.x;
    dir          = pos.x < startX ? DIR_RIGHT : DIR_LEFT;
    actor_setDir(actor, dir);
    actor_moveNoCheck(actor, dir, frameTime);
    if (fabsf(pos.x - startX) < slop) {
      actor_setPos(actor, (Vector2) { startX, pos.y });
      actor_setDir(actor, creature_START_DIR);
      actor_setSpeed(actor, creature_getSpeed(creature));
      creature->update = g_state.update;
    }
  }
}

// Creature moves from maze start back to pen
void creature_startToPen(creature_Creature* creature, float frameTime, float slop) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game_Actor* actor = creature->actor;
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
      actor_setSpeed(actor, SPEED_SLOW);
      actor_setDir(actor, DIR_UP);
      creature->update = creature_pen;
    }
  }
}

// Creature wanders randomly
void creature_frightened(creature_Creature* creature, float frameTime, float slop) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  creatureUpdateCommon(creature, frameTime, slop, &FrightenedHandler);
}

// Creature chases player
void creature_chase(creature_Creature* creature, float frameTime, float slop) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  creatureUpdateCommon(creature, frameTime, slop, &ChaseHandler);
}

// Creature heads to it's assigned corner
void creature_scatter(creature_Creature* creature, float frameTime, float slop) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  creatureUpdateCommon(creature, frameTime, slop, &ScatterHandler);
}

// Creature heads back to the pen
void creature_dead(creature_Creature* creature, float frameTime, float slop) {
  assert(creature != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  creatureUpdateCommon(creature, frameTime, slop, &DeadHandler);

  Vector2   pos       = actor_getPos(creature->actor);
  game_Tile startTile = maze_getTile(creature->mazeStart);
  Vector2   dest      = { startTile.col * TILE_SIZE, startTile.row * TILE_SIZE };
  if (fabsf(pos.x - dest.x) < slop && fabsf(pos.y - dest.y) < slop) {
    creature->update = creature_startToPen;
    audio_stopWhispers(&creature->whisperId);
  }
}
