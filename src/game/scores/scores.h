// clang-format Language: C
#pragma once

// --- Types ---

typedef struct scores_Result {
  bool isTimeRecord;
  bool isScoreRecord;
} scores_Result;

// --- Score functions ---

void          scores_load(void);
void          scores_save(void);
scores_Result scores_levelClear(double time, int score);
scores_Result scores_fullRun(double time, int score);
