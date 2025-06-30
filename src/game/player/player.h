// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"

// --- Player functions ---

bool             player_init(void);
void             player_shutdown(void);
void             player_update(float frameTime, float slop);
Vector2          player_getPos(void);
game_Dir         player_getDir(void);
bool             player_isMoving(void);
game__Actor*     player_getActor(void);
game_Tile        player_tileAhead(int tileNum);
float            player_getMaxSpeed(void);
void             player_dead(void);
void             player_reset(void);
void             player_restart(void);
void             player_totalReset(void);
int              player_getLives(void);
int              player_getScore(void);
game_PlayerState player_getState(void);
bool             player_hasSword(void);
float            player_getSwordTimer(void);
void             player_killedGhost(int ghostID);
int              player_getCoinsCollected(void);
