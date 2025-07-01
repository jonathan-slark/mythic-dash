// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"

// --- Actor functions ---

game_Actor* actor_create(Vector2 pos, Vector2 size, game_Dir dir, float speed);
void        actor_destroy(game_Actor** actor);
Vector2     actor_getPos(const game_Actor* actor);
Vector2     actor_getCentre(const game_Actor* actor);
void        actor_setPos(game_Actor* actor, Vector2 pos);
Vector2     actor_getSize(const game_Actor* actor);
game_Dir    actor_getDir(const game_Actor* actor);
void        actor_setDir(game_Actor* actor, game_Dir dir);
bool        actor_isMoving(const game_Actor* actor);
void        actor_startMoving(game_Actor* actor);
game_AABB   actor_getAABB(const game_Actor* actor);
bool        actor_canMove(game_Actor* actor, game_Dir dir, float slop);
void        actor_setSpeed(game_Actor* actor, float speed);
float       actor_getSpeed(game_Actor* actor);
void        actor_overlay(const game_Actor* actor, Color colour);
void        actor_moveOverlay(game_Actor* actor);
void        actor_canMoveOverlay(game_Actor* actor);
void        actor_moveNoCheck(game_Actor* actor, game_Dir dir, float frameTime);
void        actor_move(game_Actor* actor, game_Dir dir, float frameTime);
void        actor_update(game_Actor* actor, float frameTime);
game_Tile   actor_nextTile(game_Actor* actor, game_Dir dir);
bool        actor_hasTeleported(game_Actor* actor);
bool        actor_isColliding(const game_Actor* actor1, const game_Actor* actor2);
