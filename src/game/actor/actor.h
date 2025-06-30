// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"

// --- Actor functions ---

game__Actor* actor_create(Vector2 pos, Vector2 size, game_Dir dir, float speed);
void         actor_destroy(game__Actor** actor);
Vector2      actor_getPos(const game__Actor* actor);
Vector2      actor_getCentre(const game__Actor* actor);
void         actor_setPos(game__Actor* actor, Vector2 pos);
Vector2      actor_getSize(const game__Actor* actor);
game_Dir     actor_getDir(const game__Actor* actor);
void         actor_setDir(game__Actor* actor, game_Dir dir);
bool         actor_isMoving(const game__Actor* actor);
void         actor_startMoving(game__Actor* actor);
game_AABB    actor_getAABB(const game__Actor* actor);
bool         actor_canMove(game__Actor* actor, game_Dir dir, float slop);
void         actor_setSpeed(game__Actor* actor, float speed);
float        actor_getSpeed(game__Actor* actor);
void         actor_overlay(const game__Actor* actor, Color colour);
void         actor_moveOverlay(game__Actor* actor);
void         actor_canMoveOverlay(game__Actor* actor);
void         actor_moveNoCheck(game__Actor* actor, game_Dir dir, float frameTime);
void         actor_move(game__Actor* actor, game_Dir dir, float frameTime);
void         actor_update(game__Actor* actor, float frameTime);
game_Tile    actor_nextTile(game__Actor* actor, game_Dir dir);
bool         actor_hasTeleported(game__Actor* actor);
bool         actor_isColliding(const game__Actor* actor1, const game__Actor* actor2);
