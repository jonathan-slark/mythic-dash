// clang-format Language: C
#pragma once

#include <raylib.h>

void game_load(void);
void game_update(void);
void game_draw(void);
void game_unload(void);

void    game_playerInit(void);
void    game_playerUpdate(void);
Vector2 game_playerGetPos(void);
