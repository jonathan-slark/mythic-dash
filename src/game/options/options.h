// clang-format Language: C
#pragma once

#include <engine/engine.h>

// --- Options functions ---

void options_load(void);
void options_save(void);

// --- Volume options ---

float options_getMasterVolume(void);
float options_getMusicVolume(void);
float options_getSfxVolume(void);

void options_setMasterVolume(float volume);
void options_setMusicVolume(float volume);
void options_setSfxVolume(float volume);

// --- Display options ---

engine_WindowMode options_getWindowMode(void);
int               options_getScreenScale(void);

void options_setWindowMode(engine_WindowMode mode);
void options_setScreenSacle(int scale);
