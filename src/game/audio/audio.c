#include "audio.h"
#include <raylib.h>
#include "../asset/asset.h"
#include "../internal.h"

// --- Constants ---

constexpr int    COIN_PITCH_COUNT = 5;
static const int COIN_THRESHOLD   = 40;

// --- Types ---

typedef struct audio_State {
  const float coinPitches[COIN_PITCH_COUNT];
  int         coinPitchIndex;
  int         coinsCollected;

  bool audioEnabled;
} audio_State;

// --- Global state ---

static audio_State g_state = {
  .coinPitches = {
      0.7937f,  // -4 semitones
      0.8409f,  // -3 semitones
      0.8909f,  // -2 semitones
      0.9439f,  // -1 semitone
      1.0000f   // root
  },
  .coinPitchIndex = 0,
  .coinsCollected = 0,
  .audioEnabled = true
};

void audo_playChime(void) {
  if (!g_state.audioEnabled) return;

  if (++g_state.coinsCollected >= COIN_THRESHOLD) {
    g_state.coinPitchIndex = MIN(g_state.coinPitchIndex + 1, COIN_PITCH_COUNT);
    g_state.coinsCollected = 0;
  }

  engine_Sound* sound = asset_getChimeSound();
  engine_setSoundPitch(sound, g_state.coinPitches[g_state.coinPitchIndex]);
  engine_playSound(sound);
}

void audio_resetChimePitch(void) {
  g_state.coinPitchIndex = 0;
  g_state.coinsCollected = 0;
}

void audio_playDeath(void) { engine_playSound(asset_getDeathSound()); }
