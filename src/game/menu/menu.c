#include "menu.h"
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include "../draw/draw.h"

// --- Types ---

typedef enum menu_Screen { MENU_MAIN, MENU_GAME, MENU_CREDITS, MENU_OPTIONS } menu_Screen;
typedef enum menu_Buttons { BUTTON_START, BUTTON_OPTIONS, BUTTON_CREDITS, BUTTON_QUIT, BUTTON_COUNT } menu_Buttons;

typedef struct {
  Rectangle   bounds;
  const char* text;
  bool        isHovered;
} menu_Button;

typedef struct {
  Rectangle   bounds;
  const char* text;
  float       value;  // 0.0 to 1.0
} menu_Slider;

typedef struct {
  Rectangle   bounds;
  const char* text;
  bool        isChecked;
} menu_Toggle;

typedef struct menu_State {
  menu_Screen screen;
  menu_Button buttons[BUTTON_COUNT];
} menu_State;

// --- Constants ---

static const Color TEXT_NORMAL = { 200, 200, 200, 255 };
static const Color TEXT_ACTIVE = { 255, 255, 255, 255 };

// --- Global state ---

menu_State g_menuState = {
  .screen  = MENU_MAIN,
  .buttons = { { .bounds = { 200, 70, 100, 10 }, .text = "Start Game", .isHovered = false },
              { .bounds = { 200, 80, 100, 10 }, .text = "Options", .isHovered = false },
              { .bounds = { 200, 90, 100, 10 }, .text = "Credits", .isHovered = false },
              { .bounds = { 200, 100, 100, 10 }, .text = "Quit Game", .isHovered = false } }
};

// --- Helper functions ---

void updateMainMenu(void) {
  g_menuState.buttons[BUTTON_START].isHovered   = engine_isMouseHover(g_menuState.buttons[BUTTON_START].bounds);
  g_menuState.buttons[BUTTON_OPTIONS].isHovered = engine_isMouseHover(g_menuState.buttons[BUTTON_OPTIONS].bounds);
  g_menuState.buttons[BUTTON_CREDITS].isHovered = engine_isMouseHover(g_menuState.buttons[BUTTON_CREDITS].bounds);
  g_menuState.buttons[BUTTON_QUIT].isHovered    = engine_isMouseHover(g_menuState.buttons[BUTTON_QUIT].bounds);

  if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_START].bounds)) {
    g_menuState.screen = MENU_GAME;
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_OPTIONS].bounds)) {
    g_menuState.screen = MENU_OPTIONS;
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_CREDITS].bounds)) {
    g_menuState.screen = MENU_CREDITS;
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_QUIT].bounds)) {
    engine_requestClose();
  }
}

void updateGameMenu(void) {}

void updateOptionsMenu(void) {}

void updateCreditsMenu(void) {}

void drawButton(menu_Button button) {
  draw_Text text = {
    .xPos     = button.bounds.x,
    .yPos     = button.bounds.y,
    .format   = button.text,
    .colour   = button.isHovered ? TEXT_ACTIVE : TEXT_NORMAL,
    .fontSize = FONT_NORMAL
  };
  draw_text(text);
}

void drawMainMenu(void) {
  drawButton(g_menuState.buttons[BUTTON_START]);
  drawButton(g_menuState.buttons[BUTTON_OPTIONS]);
  drawButton(g_menuState.buttons[BUTTON_CREDITS]);
  drawButton(g_menuState.buttons[BUTTON_QUIT]);
}

void drawGameMenu(void) {}

void drawOptionsMenu(void) {}

void drawCreditsMenu(void) {}

// --- Menu functions ---

void menu_update(void) {
  switch (g_menuState.screen) {
    case MENU_MAIN: updateMainMenu(); break;
    case MENU_OPTIONS: updateOptionsMenu(); break;
    case MENU_CREDITS: updateCreditsMenu(); break;
    case MENU_GAME: updateGameMenu(); break;
  }
}

void menu_draw(void) {
  switch (g_menuState.screen) {
    case MENU_MAIN: drawMainMenu(); break;
    case MENU_OPTIONS: drawOptionsMenu(); break;
    case MENU_CREDITS: drawCreditsMenu(); break;
    case MENU_GAME: drawGameMenu(); break;
  }
}
