// clang-format Language: C
#pragma once

#include <engine/engine.h>
#include <raylib.h>
#include "../internal.h"

// --- Constants ---

static const int MAX_SOUNDS = 10;

// --- Asset functions

bool asset_load(void);
void asset_unload(void);
bool asset_initPlayer(void);
bool asset_initCreatures(void);
bool asset_initCursor(void);
void asset_shutdownPlayer(void);
void asset_shutdownCreatures(void);
void asset_shutdownCursor(void);

engine_Texture* asset_getCreatureSpriteSheet(void);
engine_Texture* asset_getPlayerSpriteSheet(void);
engine_Sprite*  asset_getPlayerSprite(game_PlayerState state);
engine_Sprite*  asset_getPlayerLivesSprite(int life);
Vector2         asset_getPlayerLivesSpritePos(int life);
engine_Sprite*  asset_getPlayerNextLifeSprite(void);
engine_Anim*    asset_getPlayerAnim(game_PlayerState state, game_Dir dir);
engine_Sprite*  asset_getCreateSprite(int creatureID);
engine_Anim*    asset_getCreatureAnim(int creatureID, game_Dir dir);
Vector2         asset_getCreatureOffset(int creatureID);
engine_Texture* asset_getCursorSpriteSheet(void);
engine_Sprite*  asset_getCursorSprite(void);
engine_Font*    asset_getFont(void);
engine_Font*    asset_getFontTiny(void);
engine_Sound*   asset_getWailSound(int id);
engine_Sound*   asset_getChimeSound(void);
engine_Sound*   asset_getDeathSound(void);
engine_Sound*   asset_getFallingSound(void);
engine_Sound*   asset_getWhispersSound(void);
engine_Sound*   asset_getPickupSound(void);
engine_Sound*   asset_getTwinkleSound(void);
engine_Sound*   asset_getWinSound(void);
engine_Sound*   asset_getGameOverSound(void);
engine_Sound*   asset_getLifeSound(void);
engine_Sound*   asset_getResSound(void);
engine_Music*   asset_getMusic(void);
void            asset_updateMusicDucking(float volume);
