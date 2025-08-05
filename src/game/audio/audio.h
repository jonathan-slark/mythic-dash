// clang-format Language: C
#pragma once

#include <raylib.h>

void audio_startMusic(void);
void audio_stopMusic(void);
void audio_playChime(Vector2 pos);
void audio_resetChimePitch(void);
void audio_playDeath(Vector2 pos);
void audio_playFalling(Vector2 pos);
void audio_playWail(Vector2 pos);
int  audio_playWhispers(Vector2 pos);
void audio_updateWhispers(int id);
void audio_stopWhispers(int* id);
void audio_playPickup(Vector2 pos);
void audio_playTwinkle(Vector2 pos);
void audio_playWin(Vector2 pos);
void audio_playGameOver(Vector2 pos);
void audio_playLife(Vector2 pos);
void audio_playRes(Vector2 pos);
