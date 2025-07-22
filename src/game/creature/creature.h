// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"

// --- Creature functions ---

bool        creature_init(void);
void        creature_shutdown(void);
void        creature_update(double frameTime, float slop);
Vector2     creature_getPos(int id);
game_Dir    creature_getDir(int id);
game_Actor* creature_getActor(int id);
float       creature_getDecisionCooldown(int id);
const char* creature_getGlobalStateString(void);
const char* creature_getStateString(int id);
game_Tile   creature_getTarget(int id);
float       creature_getGlobalTimer(void);
int         creature_getGlobaStateNum(void);
void        creature_reset(void);
void        creature_swordPickup(void);
void        creature_swordDrop(void);
bool        creature_isFrightened(int id);
bool        creature_isDead(int id);
void        creature_setScore(int id, int score);
int         creature_getScore(int id);
