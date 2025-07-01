// clang-format Language: C
#pragma once

#include <engine/engine.h>
#include <raylib.h>
#include "../internal.h"

// --- Asset functions

bool asset_load(void);
void asset_unload(void);
bool asset_initPlayer(void);
bool asset_initCreatures(void);
void asset_shutdownPlayer(void);
void asset_shutdownCreatures(void);

engine_Texture* asset_getCreatureSpriteSheet(void);
engine_Texture* asset_getPlayerSpriteSheet(void);
engine_Sprite*  asset_getPlayerSprite(game_PlayerState state);
engine_Sprite*  asset_getPlayerLivesSprite(int life);
engine_Sprite*  asset_getPlayerNextLifeSprite(void);
engine_Anim*    asset_getPlayerAnim(game_PlayerState state, game_Dir dir);
engine_Sprite*  asset_getCreateSprite(int creatureID);
engine_Anim*    asset_getCreatureAnim(int creatureID, game_Dir dir);
engine_Font*    asset_getFont(void);
engine_Font*    asset_getFontTiny(void);
Vector2         asset_getCreatureOffset(int creatureID);
