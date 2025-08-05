#include "audio.h"
#include <assert.h>
#include <raylib.h>
#include "../asset/asset.h"
#include "../internal.h"
#include "../maze/maze.h"
#include "engine/engine.h"

// --- Constants ---

constexpr int    COIN_PITCH_COUNT = 5;
static const int COIN_THRESHOLD   = 20;

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

// --- Helper functions ---

static inline float getPan(Vector2 pos) { return 1.0f - pos.x / (maze_getCols() * TILE_SIZE); }

// --- Audio functions ----

void audio_startMusic(void) { engine_playMusic(asset_getMusic()); }

void audio_stopMusic(void) { engine_stopMusic(asset_getMusic()); }

void audio_playChime(Vector2 pos) {
  if (!g_state.audioEnabled) return;

  if (++g_state.coinsCollected >= COIN_THRESHOLD) {
    g_state.coinPitchIndex = MIN(g_state.coinPitchIndex + 1, COIN_PITCH_COUNT);
    g_state.coinsCollected = 0;
  }

  engine_Sound* sound = asset_getChimeSound();
  engine_setSoundPitch(sound, g_state.coinPitches[g_state.coinPitchIndex]);
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

void audio_resetChimePitch(void) {
  g_state.coinPitchIndex = 0;
  g_state.coinsCollected = 0;
}

void audio_playDeath(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getDeathSound();
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

void audio_playFalling(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getFallingSound();
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

void audio_playWail(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getWailSound(GetRandomValue(0, WAIL_SOUND_COUNT - 1));
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

int audio_playWhispers(Vector2 pos) {
  if (!g_state.audioEnabled) return -1;
  engine_Sound* sound = asset_getWhispersSound();
  engine_setSoundPan(sound, getPan(pos));
  return engine_playSound(sound);
}

void audio_updateWhispers(int id) {
  if (!g_state.audioEnabled) return;
  assert(id >= 0 && id < MAX_SOUNDS);
  engine_Sound* sound = asset_getWhispersSound();
  engine_updateSound(sound, id);
}

void audio_stopWhispers(int* id) {
  if (!g_state.audioEnabled) return;
  assert(id != nullptr && *id >= 0 && *id < MAX_SOUNDS);
  engine_Sound* sound = asset_getWhispersSound();
  engine_stopSound(sound, *id);
  *id = -1;
}

void audio_playPickup(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getPickupSound();
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

void audio_playTwinkle(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getTwinkleSound();
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

void audio_playWin(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getWinSound();
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

void audio_playGameOver(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getGameOverSound();
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

void audio_playLife(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getLifeSound();
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}

void audio_playRes(Vector2 pos) {
  if (!g_state.audioEnabled) return;
  engine_Sound* sound = asset_getResSound();
  engine_setSoundPan(sound, getPan(pos));
  engine_playSound(sound);
}
