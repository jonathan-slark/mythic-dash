// clang-format Language: C
#pragma once

#include <game/game.h>

// --- Types ---

typedef struct scores_Result {
  bool isTimeRecord;
  bool isScoreRecord;
} scores_Result;

// --- Score functions ---

void          scores_load(void);
void          scores_save(void);
scores_Result scores_levelClear(double time, int score, int lives);
scores_Result scores_fullRun(double time, int score, int lives);
const char*   scores_printTime(double time);
void          scores_drawMenu(void);
void          scores_setEasy(void);
void          scores_setNormal(void);
void          scores_setArcade(void);
void          scores_setScore(void);
void          scores_setTime(void);
