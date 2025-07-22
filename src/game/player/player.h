// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"
#include "../scores/scores.h"

typedef struct player_levelData {
  float                   levelTimer;
  int                     levelScore;
  scores_LevelClearResult levelClearResult;
} player_levelData;

// --- Player functions ---

bool             player_init(void);
void             player_shutdown(void);
void             player_update(float frameTime, float slop);
Vector2          player_getPos(void);
game_Dir         player_getDir(void);
bool             player_isMoving(void);
game_Actor*      player_getActor(void);
game_Tile        player_tileAhead(int tileNum);
float            player_getMaxSpeed(void);
void             player_dead(void);
void             player_reset(void);
player_levelData player_getLevelData(void);
void             player_restart(void);
void             player_totalReset(void);
int              player_getLives(void);
int              player_getScore(void);
game_PlayerState player_getState(void);
bool             player_hasSword(void);
float            player_getSwordTimer(void);
void             player_killedCreature(int creatureID);
int              player_getCoinsCollected(void);
int              player_getNextExtraLifeScore(void);
