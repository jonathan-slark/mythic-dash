// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"

// --- Ghost functions ---

bool         ghost_init(void);
void         ghost_shutdown(void);
void         ghost_update(float frameTime, float slop);
Vector2      ghost_getPos(int id);
game__Dir    ghost_getDir(int id);
game__Actor* ghost_getActor(int id);
float        ghost_getDecisionCooldown(int id);
const char*  ghost_getGlobalStateString(void);
const char*  ghost_getStateString(int id);
game__Tile   ghost_getTarget(int id);
float        ghost_getGlobalTimer(void);
int          ghost_getGlobaStateNum(void);
void         ghost_reset(void);
void         ghost_swordPickup(void);
void         ghost_swordDrop(void);
bool         ghost_isFrightened(int id);
bool         ghost_isDead(int id);
void         ghost_setScore(int id, int score);
int          ghost_getScore(int id);
