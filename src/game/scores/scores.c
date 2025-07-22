#include "scores.h"
#include <assert.h>
#include <errno.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "../internal.h"
#include "log/log.h"

// --- Types ---

constexpr int ENTRY_LEN  = 16;
constexpr int BUFFER_LEN = 256;

typedef struct {
  char  difficulty[ENTRY_LEN];
  int   level;
  float time;
  int   score;
  int   lives;
  char  type[ENTRY_LEN];
} score_Entry;

typedef struct {
  score_Entry bestTimes[DIFFICULTY_COUNT][LEVEL_COUNT];
  score_Entry bestScores[DIFFICULTY_COUNT][LEVEL_COUNT];
  score_Entry fullRunsBestTimes[DIFFICULTY_COUNT];
  score_Entry fullRunsBestScores[DIFFICULTY_COUNT];
} score_Saves;

// --- Constants ---

static const char  SCORES_FILE[]                = "scores.csv";
static const int   ENTRY_COUNT                  = 5;
static const char  TYPE_TIME[]                  = "time";
static const char  TYPE_SCORE[]                 = "score";
static const char  TYPE_FULL_TIME[]             = "fullTime";
static const char  TYPE_FULL_SCORE[]            = "fullScore";
static const char* MODE_NAMES[DIFFICULTY_COUNT] = { "Easy", "Normal", "Arcade" };

// --- Global state ---

static score_Saves g_saves = {};

// --- Helper functions ---

static game_Difficulty getDifficulty(const char difficulty[]) {
  for (game_Difficulty i = 0; i < DIFFICULTY_COUNT; i++) {
    if (strcmp(difficulty, MODE_NAMES[i]) == 0) return i;
  }

  assert(false);
}

// --- Score functions ---

void scores_load(void) {
  FILE* file = fopen(SCORES_FILE, "r");
  if (file == nullptr) {
    LOG_WARN(game_log, "Could not open file %s (%s)", SCORES_FILE, strerror(errno));
    return;
  }

  score_Entry entry;
  char        line[BUFFER_LEN];
  while (fgets(line, sizeof(line), file) != nullptr) {
    int returnValue = sscanf(
        line,
        "%15[^,],%d,%f,%d,%d,%15[^,\n]",
        entry.difficulty,
        &entry.level,
        &entry.time,
        &entry.score,
        &entry.lives,
        entry.type
    );
    if (returnValue < ENTRY_COUNT) {
      LOG_ERROR(game_log, "Unable to process %s", SCORES_FILE);
      return;
    }

    if (strcmp(entry.type, TYPE_TIME) == 0) {
      g_saves.bestTimes[getDifficulty(entry.difficulty)][entry.level] = entry;
    } else if (strcmp(entry.type, TYPE_SCORE) == 0) {
      g_saves.bestScores[getDifficulty(entry.difficulty)][entry.level] = entry;
    } else if (strcmp(entry.type, TYPE_FULL_TIME) == 0) {
      g_saves.fullRunsBestTimes[getDifficulty(entry.difficulty)] = entry;
    } else if (strcmp(entry.type, TYPE_FULL_SCORE) == 0) {
      g_saves.fullRunsBestScores[entry.level] = entry;
    } else {
      LOG_ERROR(game_log, "Syntax error in %s (%s)", SCORES_FILE, entry.type);
      return;
    }
  }
  if (errno > 0) LOG_ERROR(game_log, "%s", strerror(errno));

  if (fclose(file) == EOF) LOG_ERROR(game_log, "Error on closing file %s (%s)", SCORES_FILE, strerror(errno));
}

void scores_save(void) {
  FILE* file = fopen(SCORES_FILE, "w");
  if (file == nullptr) {
    LOG_WARN(game_log, "Could not open file for writing %s (%s)", SCORES_FILE, strerror(errno));
    return;
  }

  for (int i = 0; i < DIFFICULTY_COUNT; i++) {
    for (int j = 0; j < LEVEL_COUNT; j++) {
      fprintf(
          file,
          "%s,%d,%.2f,%d,%d,%s\n",
          MODE_NAMES[i],
          g_saves.bestTimes[i][j].level = j + 1,
          g_saves.bestTimes[i][j].time,
          g_saves.bestTimes[i][j].score,
          g_saves.bestTimes[i][j].lives,
          TYPE_TIME
      );
      fprintf(
          file,
          "%s,%d,%.2f,%d,%d,%s\n",
          MODE_NAMES[i],
          g_saves.bestTimes[i][j].level = j + 1,
          g_saves.bestScores[i][j].time,
          g_saves.bestScores[i][j].score,
          g_saves.bestScores[i][j].lives,
          TYPE_SCORE
      );
    }
  }

  if (fclose(file) == EOF) LOG_ERROR(game_log, "Error on closing file %s (%s)", SCORES_FILE, strerror(errno));
}

void scores_levelClear(float time, int score) {
  int             level      = game_getLevel();
  game_Difficulty difficulty = game_getDifficulty();
  if (time > g_saves.bestTimes[difficulty][level].time) {
    g_saves.bestTimes[difficulty][level].time = time;
  }
  if (score > g_saves.bestScores[difficulty][level].score) {
    g_saves.bestScores[difficulty][level].score = score;
  }
}
