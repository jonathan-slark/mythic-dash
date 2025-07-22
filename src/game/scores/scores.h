// clang-format Language: C
#pragma once

// --- Types ---

typedef struct scores_LevelClearResult {
  bool isTimeRecord;
  bool isScoreRecord;
} scores_LevelClearResult;

// --- Score functions ---

void                    scores_load(void);
void                    scores_save(void);
scores_LevelClearResult scores_levelClear(float time, int score);
