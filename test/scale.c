#include <math.h>
#include <stdio.h>

// --- Constants ---

static const float NORMAL_SPEED_MIN_MULT = 0.75f;
static const float NORMAL_SPEED_MAX_MULT = 0.95f;
static const float FRIGHT_SPEED_MIN_MULT = 0.50f;
static const float FRIGHT_SPEED_MAX_MULT = 0.60f;
static const float PLAYER_MAX_SPEED      = 60.0f;

// --- Global state ---

bool g_playerHasSword = false;
int  g_level          = 1;

// --- Mocked functions ---

static bool player_hasSword(void) { return g_playerHasSword; }

static int game_getLevel(void) { return g_level++; }

static int player_getMaxSpeed(void) { return PLAYER_MAX_SPEED; }

// --- Functions to test ---

static inline float ghost__getSpeed(void) {
  if (player_hasSword()) {
    return fminf(FRIGHT_SPEED_MIN_MULT + (game_getLevel() - 1) * 0.0055f, FRIGHT_SPEED_MAX_MULT) * player_getMaxSpeed();
  } else {
    return fminf(NORMAL_SPEED_MIN_MULT + (game_getLevel() - 1) * 0.011f, NORMAL_SPEED_MAX_MULT) * player_getMaxSpeed();
  }
}

static float getSwordTimer(void) {
  const float max   = 6.0f;
  const float min   = 3.6f;
  const int   level = game_getLevel();
  float       t     = fminf(fmaxf((level - 1) / 19.0f, 0.0f), 1.0f);
  t                 = 1.0f - powf(1.0f - t, 2.0f);  // ease-out curve
  return max - t * (max - min);
}

// --- Helper functions ---

static void printGhostSpeeds(void) {
  int level = 1;
  for (int i = 0; i < 20; i++) {
    float speed = ghost__getSpeed();
    fprintf(stdout, "Level: %02d, ghost speed %f\n", level++, speed);
  }
}

static void printSwordTimers(void) {
  int level = 1;
  for (int i = 0; i < 20; i++) {
    float timer = getSwordTimer();
    fprintf(stdout, "Level: %02d, sword timer %f\n", level++, timer);
  }
}

// --- Main ---

int main(void) {
  fprintf(stdout, "Normal ghost speeds:\n");
  g_playerHasSword = false;
  g_level          = 1;
  printGhostSpeeds();

  fprintf(stdout, "\nFright ghost speeds:\n");
  g_playerHasSword = true;
  g_level          = 1;
  printGhostSpeeds();

  fprintf(stdout, "\nSword timer:\n");
  g_level = 1;
  printSwordTimers();

  return 0;
}
