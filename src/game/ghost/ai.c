#include <assert.h>
#include <limits.h>
#include <math.h>
#include "../game.h"
#include "ghost.h"

// --- Helper functions ---

static inline game__Dir getOppositeDir(game__Dir dir) {
  assert(dir >= 0 && dir < DIR_COUNT);
  return (dir + 2) % DIR_COUNT;
}

static inline game__Dir randomSelect(game__Dir dirs[], int count) {
  assert(count > 0);
  return dirs[GetRandomValue(0, count - 1)];
}

static int
getValidDirs(game__Actor* actor, game__Dir currentDir, game__Dir* validDirs, bool isChangedState, float slop) {
  assert(actor != nullptr);
  assert(currentDir >= 0 && currentDir < DIR_COUNT);
  assert(validDirs != nullptr);
  assert(slop > 0.0f);

  game__Dir opposite = getOppositeDir(currentDir);
  int       count    = 0;

  for (game__Dir dir = DIR_UP; dir < DIR_COUNT; dir++) {
    if ((isChangedState || dir != opposite) && actor_canMove(actor, dir, slop)) {
      validDirs[count++] = dir;
    }
  }
  return count;
}

static game__Dir greedyDirSelect(ghost__Ghost* ghost, game__Dir dirs[], int count, game__Tile targetTile) {
  assert(count > 0);

  game__Dir bestDir = DIR_NONE;
  int       minDist = INT_MAX;
  for (int i = 0; i < count; i++) {
    game__Tile nextTile = actor_nextTile(ghost->actor, dirs[i]);
    int        dist     = maze_manhattanDistance(nextTile, targetTile);
    if (dist < minDist) {
      bestDir = dirs[i];
      minDist = dist;
    }
  }
  return bestDir;
}

static game__Tile getTargetTile(ghost__Ghost* ghost) {
  game__Tile targetTile;
  game__Tile playerTile = maze_getTile(player_getPos());
  switch (ghost->id) {
    // Blinky: directly target player's current tile
    case 1: targetTile = playerTile; break;
    // Pinky: target four tiles ahead of player, based on his current direction
    case 2: targetTile = player_tileAhead(4); break;
    // Inky: uses a vector based on both Blinky’s position and four tiles ahead of player
    case 0:
      game__Tile ghost1Tile = maze_getTile(actor_getPos(ghost_getActor(1)));
      targetTile            = maze_doubleVectorBetween(ghost1Tile, player_tileAhead(2));
      break;
    // Clyde: chases player until close, then retreats to corner
    case 3:
      if (maze_manhattanDistance(maze_getTile(actor_getPos(ghost->actor)), playerTile) < 8) {
        targetTile = ghost->cornerTile;
      } else {
        targetTile = playerTile;
      }
      break;
    default: assert(false);
  }
  return targetTile;
}

static float getSpeed() { return fminf(SPEED_MIN_MULT + game_getLevel() * 0.02f, SPEED_MAX_MULT) * player_getSpeed(); }

// --- Ghost state functions ---

// Ghost moves back and forth in pen till released
void ghost__pen(ghost__Ghost* ghost, float frameTime, float slop) {
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
    ghost->update = ghost__penToStart;
  } else {
    ghost->timer -= frameTime;
  }
}

// Ghost moves from pen to start position
void ghost__penToStart(ghost__Ghost* ghost, float frameTime, float slop) {
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
      actor_setSpeed(actor, getSpeed());
      ghost->update = ghost__scatter;
    }
  }
}

// Ghost wanders randomly
void ghost__frightened(ghost__Ghost* ghost, float frameTime, float slop) {
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
    int       count = getValidDirs(actor, currentDir, validDirs, false, slop);
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

// Ghost chases player
void ghost__chase(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor      = ghost->actor;
  game__Dir    currentDir = actor_getDir(actor);
  actor_move(actor, currentDir, frameTime);

  ghost->decisionCooldown -= frameTime;
  if (ghost->decisionCooldown < 0.0f) ghost->decisionCooldown = 0.0f;

  // Check if we hit a wall or can make a direction decision
  if (ghost->isChangedState || !actor_canMove(actor, currentDir, slop) || ghost->decisionCooldown == 0.0f) {
    game__Dir validDirs[DIR_COUNT - 1];
    int       count       = getValidDirs(actor, currentDir, validDirs, ghost->isChangedState, slop);
    ghost->isChangedState = false;
    if (count == 0) {
      // Should never happen - log error
      Vector2 pos = actor_getPos(actor);
      LOG_ERROR(game__log, "Ghost %u has no valid directions at (%.2f, %.2f)", ghost->id, pos.x, pos.y);
      actor_setDir(actor, getOppositeDir(currentDir));
      ghost->decisionCooldown = DECISION_COOLDOWN;
    } else {
      ghost->targetTile = getTargetTile(ghost);
      game__Dir bestDir = greedyDirSelect(ghost, validDirs, count, ghost->targetTile);
      // Check if we were at a junction
      if (count > 1 || currentDir != bestDir) {
        actor_setDir(actor, bestDir);
        LOG_DEBUG(game__log, "Ghost %u target tile: %d, %d", ghost->id, ghost->targetTile.col, ghost->targetTile.row);
        LOG_DEBUG(game__log, "Ghost %u chose new direction: %s", ghost->id, DIR_STRINGS[bestDir]);
        ghost->decisionCooldown = DECISION_COOLDOWN;
      }
    }
  }
}

// Ghost heads to it's assigned corners
void ghost__scatter(ghost__Ghost* ghost, float frameTime, float slop) {
  assert(ghost != nullptr);
  assert(frameTime >= 0.0f);
  assert(slop >= MIN_SLOP && slop <= MAX_SLOP);

  game__Actor* actor      = ghost->actor;
  game__Dir    currentDir = actor_getDir(actor);
  actor_move(actor, currentDir, frameTime);

  ghost->decisionCooldown -= frameTime;
  if (ghost->decisionCooldown < 0.0f) ghost->decisionCooldown = 0.0f;

  // Check if we hit a wall or can make a direction decision
  if (ghost->isChangedState || !actor_canMove(actor, currentDir, slop) || ghost->decisionCooldown == 0.0f) {
    game__Dir validDirs[DIR_COUNT - 1];
    int       count       = getValidDirs(actor, currentDir, validDirs, ghost->isChangedState, slop);
    ghost->isChangedState = false;
    if (count == 0) {
      // Should never happen - log error
      Vector2 pos = actor_getPos(actor);
      LOG_ERROR(game__log, "Ghost %u has no valid directions at (%.2f, %.2f)", ghost->id, pos.x, pos.y);
      actor_setDir(actor, getOppositeDir(currentDir));
      ghost->decisionCooldown = DECISION_COOLDOWN;
    } else {
      ghost->targetTile = ghost->cornerTile;
      game__Dir bestDir = greedyDirSelect(ghost, validDirs, count, ghost->targetTile);
      // Check if we were at a junction
      if (count > 1 || currentDir != bestDir) {
        actor_setDir(actor, bestDir);
        LOG_DEBUG(game__log, "Ghost %u target tile: %d, %d", ghost->id, ghost->targetTile.col, ghost->targetTile.row);
        LOG_DEBUG(game__log, "Ghost %u chose new direction: %s", ghost->id, DIR_STRINGS[bestDir]);
        ghost->decisionCooldown = DECISION_COOLDOWN;
      }
    }
  }
}
