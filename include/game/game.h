// clang-format Language: C
#pragma once

// --- Types ---

typedef enum game_Difficulty {
  DIFFICULTY_EASY,
  DIFFICULTY_NORMAL,
  DIFFICULTY_ARCADE,
  DIFFICULTY_COUNT
} game_Difficulty;

// --- Game functions ---

bool            game_load(void);
void            game_input(void);
void            game_update(double frameTime);
void            game_draw(void);
void            game_unload(void);
game_Difficulty game_getDifficulty(void);
