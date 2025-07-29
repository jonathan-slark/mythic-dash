// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"
#include "../scores/scores.h"
#include "game/game.h"

typedef struct player_levelData {
  double        time;
  int           score;
  int           lives;
  int           frameCount;
  scores_Result clearResult;
} player_levelData;

// --- Player functions ---

bool             player_init(void);
void             player_shutdown(void);
void             player_ready(void);
void             player_update(double frameTime, float slop);
void             player_restart(void);
void             player_reset(void);
void             player_totalReset(void);
void             player_onPause(void);
void             player_onResume(void);
game_Tile        player_tileAhead(int tileNum);
game_Actor*      player_getActor(void);
Vector2          player_getPos(void);
game_Dir         player_getDir(void);
float            player_getMaxSpeed(void);
player_levelData player_getLevelData(void);
player_levelData player_getFullRunData(void);
int              player_getLives(void);
int              player_getScore(void);
game_PlayerState player_getState(void);
float            player_getSwordTimer(void);
int              player_getCoinsCollected(void);
int              player_getNextExtraLifeScore(void);
int              player_getContinue(game_Difficulty difficulty);
void             player_drawContinue(void);
bool             player_isMoving(void);
bool             player_hasSword(void);
void             player_dead(void);
void             player_killedCreature(int creatureID);
