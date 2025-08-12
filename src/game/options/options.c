#include "options.h"
#include <engine/engine.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Constants ---

static const float             DEFAULT_VOLUME       = 1.0f;
static const engine_WindowMode DEFAULT_WINDOW_MODE  = MODE_BORDERLESS;
static const int               DEFAULT_SCREEN_SCALE = 1;
static const char              OPTIONS_FILE[]       = "options.txt";

#define MASTER_VOLUME "masterVolume"
#define MUSIC_VOLUME "musicVolume"
#define SFX_VOLUME "sfxVolume"
#define WINDOW_MODE "windowMode"
#define SCREEN_SCALE "screenScale"
static const char FORMAT_MASTER_VOLUME[] = MASTER_VOLUME "=%.2f\n";
static const char FORMAT_MUSIC_VOLUME[]  = MUSIC_VOLUME "=%.2f\n";
static const char FORMAT_SFX_VOLUME[]    = SFX_VOLUME "=%.2f\n";
static const char FORMAT_WINDOW_MODE[]   = WINDOW_MODE "=%d\n";
static const char FORMAT_SCREEN_SCALE[]  = SCREEN_SCALE "=%d\n";

constexpr int LINE_LENGTH    = 64;
constexpr int SETTING_LENGTH = 16;
constexpr int VALUE_LENGTH   = 16;
#define SETTING_NO_NULL_LENGTH "15"
#define VALUE_NO_NULL_LENGTH "15"

// --- Global state ---

static struct {
  // Volume
  float masterVolume;
  float musicVolume;
  float sfxVolume;

  // Display
  engine_WindowMode windowMode;
  int               screenScale;
} g_options;

// --- Helper functions ---

static void setDefaults(void) {
  g_options.masterVolume = DEFAULT_VOLUME;
  g_options.musicVolume  = DEFAULT_VOLUME;
  g_options.sfxVolume    = DEFAULT_VOLUME;
  g_options.windowMode   = DEFAULT_WINDOW_MODE;
  g_options.screenScale  = DEFAULT_SCREEN_SCALE;
}

static void loadOptionsFile(const char* fileName) {
  FILE* file = fopen(fileName, "r");
  if (!file) return;

  char line[LINE_LENGTH];
  while (fgets(line, sizeof(line), file)) {
    char setting[SETTING_LENGTH];
    char valueStr[VALUE_LENGTH];

    // Parse "name=value" into two strings
    if (sscanf(line, "%" SETTING_NO_NULL_LENGTH "[^=]=%" VALUE_NO_NULL_LENGTH "s", setting, valueStr) == 2) {
      // Decide whether it's int or float
      char* endPtr;
      long  asInt   = strtol(valueStr, &endPtr, 10);
      int   isInt   = (*endPtr == '\0');  // fully consumed => integer
      float asFloat = strtof(valueStr, &endPtr);
      int   isFloat = (*endPtr == '\0');  // fully consumed => float

      if (strcmp(setting, MASTER_VOLUME) == 0 && isFloat) {
        g_options.masterVolume = asFloat;
      } else if (strcmp(setting, MUSIC_VOLUME) == 0 && isFloat) {
        g_options.musicVolume = asFloat;
      } else if (strcmp(setting, SFX_VOLUME) == 0 && isFloat) {
        g_options.sfxVolume = asFloat;
      } else if (strcmp(setting, WINDOW_MODE) == 0 && isInt) {
        g_options.windowMode = (engine_WindowMode) asInt;
      } else if (strcmp(setting, SCREEN_SCALE) == 0 && isInt) {
        g_options.screenScale = asInt;
      }
    }
  }

  fclose(file);
}

static void saveOptionsFile(const char* fileName) {
  FILE* file = fopen(fileName, "w");
  if (!file) return;

  fprintf(file, FORMAT_MASTER_VOLUME, g_options.masterVolume);
  fprintf(file, FORMAT_MUSIC_VOLUME, g_options.musicVolume);
  fprintf(file, FORMAT_SFX_VOLUME, g_options.sfxVolume);
  fprintf(file, FORMAT_WINDOW_MODE, (int) g_options.windowMode);
  fprintf(file, FORMAT_SCREEN_SCALE, g_options.screenScale);

  fclose(file);
}

// --- Options functions ---

void options_load(void) {
  setDefaults();
  loadOptionsFile(OPTIONS_FILE);
}

void options_save(void) { saveOptionsFile(OPTIONS_FILE); }

// --- Volume options ---

float options_getMasterVolume(void) { return g_options.masterVolume; }
float options_getMusicVolume(void) { return g_options.musicVolume; }
float options_getSfxVolume(void) { return g_options.sfxVolume; }

void options_setMasterVolume(float volume) {
  g_options.masterVolume = volume;
  options_save();
}

void options_setMusicVolume(float volume) {
  g_options.musicVolume = volume;
  options_save();
}

void options_setSfxVolume(float volume) {
  g_options.sfxVolume = volume;
  options_save();
}

// --- Display options ---

engine_WindowMode options_getWindowMode(void) { return g_options.windowMode; }
int               options_getScreenScale(void) { return g_options.screenScale; }

void options_setWindowMode(engine_WindowMode mode) {
  g_options.windowMode = mode;
  options_save();
}

void options_setScreenSacle(int scale) {
  g_options.screenScale = scale;
  options_save();
}
