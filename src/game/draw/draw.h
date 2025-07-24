// clang-format Language: C
#pragma once

#include <raylib.h>

// --- Types ---

typedef enum draw_FontSize { FONT_TINY, FONT_NORMAL } draw_FontSize;

typedef struct draw_Text {
  const char*   format;
  int           xPos;
  int           yPos;
  Color         colour;
  draw_FontSize fontSize;
} draw_Text;

// --- Constants ---

constexpr Color TEXT_COLOUR = { 255, 255, 255, 255 };

// --- Draw functions ---

void draw_text(draw_Text text, ...);
int  draw_getTextOffset(int number);
void draw_shadowText(draw_Text text, ...);
void draw_resetPlayer(void);
void draw_updatePlayer(double frameTime, float slop);
void draw_resetCreatures(void);
void draw_updateCreatures(double frameTime, float slop);
void draw_player(void);
void draw_creatures(void);
void draw_cursor(void);
void draw_interface(void);
void draw_nextLife(void);
void draw_title(void);
void draw_ready(void);
void draw_levelClear(void);
void draw_gameOver(void);
void draw_gameWon(void);
