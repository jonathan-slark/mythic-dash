#include "scores.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "../draw/draw.h"
#include "../internal.h"
#include "game/game.h"
#include "log/log.h"

// --- Types ---

constexpr int ENTRY_LEN  = 16;
constexpr int BUFFER_LEN = 256;

typedef struct score_Entry {
  char   difficulty[ENTRY_LEN];
  int    level;
  double time;
  int    score;
  int    lives;
  char   type[ENTRY_LEN];
} score_Entry;

typedef struct score_Saves {
  score_Entry bestTimes[DIFFICULTY_COUNT][LEVEL_COUNT];
  score_Entry bestScores[DIFFICULTY_COUNT][LEVEL_COUNT];
  score_Entry fullRunsBestTimes[DIFFICULTY_COUNT];
  score_Entry fullRunsBestScores[DIFFICULTY_COUNT];
} score_Saves;

typedef enum SortBy { SORTBY_TIME, SORTBY_SCORE } SortBy;

typedef struct score_State {
  game_Difficulty difficulty;
  SortBy          sort;
} score_State;

// --- Constants ---

static const char  SCORES_FILE[]                = "scores.csv";
static const int   ENTRY_COUNT                  = 5;
static const char  TYPE_TIME[]                  = "time";
static const char  TYPE_SCORE[]                 = "score";
static const char  TYPE_FULL_TIME[]             = "fullTime";
static const char  TYPE_FULL_SCORE[]            = "fullScore";
static const char* MODE_NAMES[DIFFICULTY_COUNT] = { "Easy", "Normal", "Arcade" };
static const draw_Text
    LEVEL_SCORE_HEADER                  = { "Level           Time  Score  Lives", 138, 115, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text FULL_RUN_TIME    = { "Full Run  %10s %6d  %5d", 138, 195, TEXT_COLOUR, FONT_NORMAL };
static const int       LINE_HEIGHT      = 10;
static const int       LEVEL_SCORE_YPOS = 125;

// --- Global state ---

static score_State g_state      = { DIFFICULTY_EASY, SORTBY_TIME };
static score_Saves g_saves      = {};
static draw_Text   g_levelScore = { "Level %d   %10s %6d  %5d", 138, LEVEL_SCORE_YPOS, TEXT_COLOUR, FONT_NORMAL };

// --- Helper functions ---

static game_Difficulty getDifficulty(const char difficulty[]) {
  for (game_Difficulty i = 0; i < DIFFICULTY_COUNT; i++) {
    if (strcmp(difficulty, MODE_NAMES[i]) == 0) return i;
  }

  assert(false);
  return DIFFICULTY_NONE;
}

static void setDifficulty(game_Difficulty difficulty) { g_state.difficulty = difficulty; }

static void setSortby(SortBy sort) { g_state.sort = sort; }

// --- Score functions ---

void scores_load(void) {
  errno      = 0;
  FILE* file = fopen(SCORES_FILE, "r");
  if (file == nullptr) {
    LOG_WARN(game_log, "Could not open file %s (%s)", SCORES_FILE, strerror(errno));
    return;
  }

  score_Entry entry = {};
  char        line[BUFFER_LEN];
  while (fgets(line, sizeof(line), file) != nullptr) {
    int returnValue = sscanf(
        line,
        "%15[^,],%d,%lf,%d,%d,%15[^,\n]",
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
    entry.level--;

    if (strcmp(entry.type, TYPE_TIME) == 0) {
      g_saves.bestTimes[getDifficulty(entry.difficulty)][entry.level] = entry;
    } else if (strcmp(entry.type, TYPE_SCORE) == 0) {
      g_saves.bestScores[getDifficulty(entry.difficulty)][entry.level] = entry;
    } else if (strcmp(entry.type, TYPE_FULL_TIME) == 0) {
      g_saves.fullRunsBestTimes[getDifficulty(entry.difficulty)] = entry;
    } else if (strcmp(entry.type, TYPE_FULL_SCORE) == 0) {
      g_saves.fullRunsBestScores[getDifficulty(entry.difficulty)] = entry;
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
          "%s,%d,%.3f,%d,%d,%s\n",
          MODE_NAMES[i],
          g_saves.bestTimes[i][j].level = j + 1,
          g_saves.bestTimes[i][j].time,
          g_saves.bestTimes[i][j].score,
          g_saves.bestTimes[i][j].lives,
          TYPE_TIME
      );
      fprintf(
          file,
          "%s,%d,%.3f,%d,%d,%s\n",
          MODE_NAMES[i],
          g_saves.bestScores[i][j].level = j + 1,
          g_saves.bestScores[i][j].time,
          g_saves.bestScores[i][j].score,
          g_saves.bestScores[i][j].lives,
          TYPE_SCORE
      );
    }
    fprintf(
        file,
        "%s,%d,%.3f,%d,%d,%s\n",
        MODE_NAMES[i],
        g_saves.fullRunsBestTimes[i].level = 0,
        g_saves.fullRunsBestTimes[i].time,
        g_saves.fullRunsBestTimes[i].score,
        g_saves.fullRunsBestTimes[i].lives,
        TYPE_FULL_TIME
    );
    fprintf(
        file,
        "%s,%d,%.3f,%d,%d,%s\n",
        MODE_NAMES[i],
        g_saves.fullRunsBestScores[i].level = 0,
        g_saves.fullRunsBestScores[i].time,
        g_saves.fullRunsBestScores[i].score,
        g_saves.fullRunsBestScores[i].lives,
        TYPE_FULL_SCORE
    );
  }

  if (fclose(file) == EOF) LOG_ERROR(game_log, "Error on closing file %s (%s)", SCORES_FILE, strerror(errno));
}

scores_Result scores_levelClear(double time, int score, int lives) {
  int             level      = game_getLevel();
  game_Difficulty difficulty = game_getDifficulty();
  scores_Result   result     = {};

  if (time < g_saves.bestTimes[difficulty][level].time || g_saves.bestTimes[difficulty][level].time == 0.0f) {
    g_saves.bestTimes[difficulty][level].time  = time;
    g_saves.bestTimes[difficulty][level].score = score;
    g_saves.bestTimes[difficulty][level].lives = lives;
    result.isTimeRecord                        = true;
  }

  if (score > g_saves.bestScores[difficulty][level].score) {
    g_saves.bestScores[difficulty][level].time  = time;
    g_saves.bestScores[difficulty][level].score = score;
    g_saves.bestScores[difficulty][level].lives = lives;
    result.isScoreRecord                        = true;
  }

  return result;
}

scores_Result scores_fullRun(double time, int score, int lives) {
  game_Difficulty difficulty = game_getDifficulty();
  scores_Result   result     = {};

  if (time < g_saves.fullRunsBestTimes[difficulty].time || g_saves.fullRunsBestTimes[difficulty].time == 0.0f) {
    g_saves.fullRunsBestTimes[difficulty].time  = time;
    g_saves.fullRunsBestTimes[difficulty].score = score;
    g_saves.fullRunsBestTimes[difficulty].lives = lives;
    result.isTimeRecord                         = true;
  }

  if (score > g_saves.fullRunsBestScores[difficulty].score) {
    g_saves.fullRunsBestScores[difficulty].time  = time;
    g_saves.fullRunsBestScores[difficulty].score = score;
    g_saves.fullRunsBestScores[difficulty].lives = lives;
    result.isScoreRecord                         = true;
  }

  return result;
}

const char* scores_printTime(double time) {
  static char  buffer[BUFFER_LEN];
  const double secondsPerMinute = 60.0;
  int          minutes          = (int) time / secondsPerMinute;
  double       seconds          = fmod(time, secondsPerMinute);
  snprintf(buffer, sizeof(buffer), "%d:%06.3f", minutes, seconds);
  return buffer;
}

void scores_drawMenu(void) {
  draw_shadowText(LEVEL_SCORE_HEADER);

  g_levelScore.yPos = LEVEL_SCORE_YPOS;
  for (int level = 0; level < LEVEL_COUNT; level++) {
    float time;
    int   score;
    int   lives;
    if (g_state.sort == SORTBY_TIME) {
      time  = g_saves.bestTimes[g_state.difficulty][level].time;
      score = g_saves.bestTimes[g_state.difficulty][level].score;
      lives = g_saves.bestTimes[g_state.difficulty][level].lives;
    } else {
      time  = g_saves.bestScores[g_state.difficulty][level].time;
      score = g_saves.bestScores[g_state.difficulty][level].score;
      lives = g_saves.bestScores[g_state.difficulty][level].lives;
    }
    if (time > 0.0) draw_shadowText(g_levelScore, level + 1, scores_printTime(time), score, lives);
    g_levelScore.yPos += LINE_HEIGHT;
  }

  float time;
  int   score;
  int   lives;
  if (g_state.sort == SORTBY_TIME) {
    time  = g_saves.fullRunsBestTimes[g_state.difficulty].time;
    score = g_saves.fullRunsBestTimes[g_state.difficulty].score;
    lives = g_saves.fullRunsBestTimes[g_state.difficulty].lives;
  } else {
    time  = g_saves.fullRunsBestScores[g_state.difficulty].time;
    score = g_saves.fullRunsBestScores[g_state.difficulty].score;
    lives = g_saves.fullRunsBestScores[g_state.difficulty].lives;
  }
  if (time > 0.0) draw_shadowText(FULL_RUN_TIME, scores_printTime(time), score, lives);
}

void scores_setEasy(void) { setDifficulty(DIFFICULTY_EASY); }

void scores_setNormal(void) { setDifficulty(DIFFICULTY_NORMAL); }

void scores_setArcade(void) { setDifficulty(DIFFICULTY_ARCADE); }

void scores_setScore(void) { setSortby(SORTBY_SCORE); }

void scores_setTime(void) { setSortby(SORTBY_TIME); }
