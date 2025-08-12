#include "options.h"
#include <engine/engine.h>
#include <raylib.h>

// --- Constants ---

static const float             DEFAULT_VOLUME      = 1.0f;
static const engine_WindowMode DEFAULT_WINDOW_MODE = MODE_BORDERLESS;

// --- Global state ---

static struct {
  // Volume
  float masterVolume;
  float musicVolume;
  float sfxVolume;

  // Display
  engine_WindowMode windowMode;
  int               windowScale;
} g_options;

// --- Options functions ---

void options_load(void) {
  g_options.masterVolume = DEFAULT_VOLUME;
  g_options.musicVolume  = DEFAULT_VOLUME;
  g_options.sfxVolume    = DEFAULT_VOLUME;
  g_options.windowMode   = DEFAULT_WINDOW_MODE;
  g_options.windowScale  = engine_getMaxScale();
}

void options_save(void) {}

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
int               options_getScreenSacle(void) { return g_options.windowScale; }

void options_setWindowMode(engine_WindowMode mode) {
  g_options.windowMode = mode;
  options_save();
}

void options_setScreenSacle(int scale) {
  g_options.windowScale = scale;
  options_save();
}
