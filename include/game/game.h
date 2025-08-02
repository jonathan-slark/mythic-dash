// clang-format Language: C
#pragma once

// --- Types ---

typedef enum game_Difficulty {
  DIFFICULTY_EASY,
  DIFFICULTY_NORMAL,
  DIFFICULTY_ARCADE,
  DIFFICULTY_COUNT,
  DIFFICULTY_NONE
} game_Difficulty;

// --- Constants ---

static const double TARGET_FPS = 60.0;  // For Arcade Mode
static const double FRAME_TIME = 1.0 / TARGET_FPS;

// --- Global state ---

extern double g_accumulator;

// --- Game functions ---

bool            game_load(void);
void            game_input(void);
void            game_update(double frameTime);
void            game_draw(void);
void            game_unload(void);
game_Difficulty game_getDifficulty(void);
