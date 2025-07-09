#include "menu.h"
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include "../draw/draw.h"
#include "../internal.h"

// --- Types ---

typedef enum menu_Screen { MENU_MAIN, MENU_GAME, MENU_OPTIONS, MENU_CREDITS } menu_Screen;
typedef enum menu_Buttons {
  BUTTON_GAME,
  BUTTON_OPTIONS,
  BUTTON_CREDITS,
  BUTTON_QUIT,
  BUTTON_EASY,
  BUTTON_NORMAL,
  BUTTON_ARCADE,
  BUTTON_BACK,
  BUTTON_COUNT
} menu_Buttons;

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
              { .bounds = { 200, 100, 100, 10 }, .text = "Quit Game", .isHovered = false },
              { .bounds = { 200, 70, 100, 10 }, .text = "Easy", .isHovered = false },
              { .bounds = { 200, 80, 100, 10 }, .text = "Normal", .isHovered = false },
              { .bounds = { 200, 90, 100, 10 }, .text = "Arcade", .isHovered = false },
              { .bounds = { 200, 100, 100, 10 }, .text = "Back", .isHovered = false } }
};

// --- Helper functions ---

void updateMainMenu(void) {
  g_menuState.buttons[BUTTON_GAME].isHovered    = engine_isMouseHover(g_menuState.buttons[BUTTON_GAME].bounds);
  g_menuState.buttons[BUTTON_OPTIONS].isHovered = engine_isMouseHover(g_menuState.buttons[BUTTON_OPTIONS].bounds);
  g_menuState.buttons[BUTTON_CREDITS].isHovered = engine_isMouseHover(g_menuState.buttons[BUTTON_CREDITS].bounds);
  g_menuState.buttons[BUTTON_QUIT].isHovered    = engine_isMouseHover(g_menuState.buttons[BUTTON_QUIT].bounds);

  if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_GAME].bounds)) {
    g_menuState.screen = MENU_GAME;
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_OPTIONS].bounds)) {
    g_menuState.screen = MENU_OPTIONS;
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_CREDITS].bounds)) {
    g_menuState.screen = MENU_CREDITS;
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_QUIT].bounds)) {
    engine_requestClose();
  }
}

void updateGameMenu(void) {
  g_menuState.buttons[BUTTON_EASY].isHovered   = engine_isMouseHover(g_menuState.buttons[BUTTON_EASY].bounds);
  g_menuState.buttons[BUTTON_NORMAL].isHovered = engine_isMouseHover(g_menuState.buttons[BUTTON_NORMAL].bounds);
  g_menuState.buttons[BUTTON_ARCADE].isHovered = engine_isMouseHover(g_menuState.buttons[BUTTON_ARCADE].bounds);
  g_menuState.buttons[BUTTON_BACK].isHovered   = engine_isMouseHover(g_menuState.buttons[BUTTON_BACK].bounds);

  if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_GAME].bounds)) {
    game_new();  // TODO: Add difficulty
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_NORMAL].bounds)) {
    game_new();  // TODO: Add difficulty
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_ARCADE].bounds)) {
    game_new();  // TODO: Add difficulty
  } else if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_BACK].bounds)) {
    g_menuState.screen = MENU_MAIN;
  }
}

void updateOptionsMenu(void) {
  g_menuState.buttons[BUTTON_BACK].isHovered = engine_isMouseHover(g_menuState.buttons[BUTTON_BACK].bounds);
  if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_BACK].bounds)) {
    g_menuState.screen = MENU_MAIN;
  }
}

void updateCreditsMenu(void) {
  g_menuState.buttons[BUTTON_BACK].isHovered = engine_isMouseHover(g_menuState.buttons[BUTTON_BACK].bounds);
  if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, g_menuState.buttons[BUTTON_BACK].bounds)) {
    g_menuState.screen = MENU_MAIN;
  }
}

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
  drawButton(g_menuState.buttons[BUTTON_GAME]);
  drawButton(g_menuState.buttons[BUTTON_OPTIONS]);
  drawButton(g_menuState.buttons[BUTTON_CREDITS]);
  drawButton(g_menuState.buttons[BUTTON_QUIT]);
}

void drawGameMenu(void) {
  drawButton(g_menuState.buttons[BUTTON_EASY]);
  drawButton(g_menuState.buttons[BUTTON_NORMAL]);
  drawButton(g_menuState.buttons[BUTTON_ARCADE]);
  drawButton(g_menuState.buttons[BUTTON_BACK]);
}

void drawOptionsMenu(void) {
  // TODO: Add options
  drawButton(g_menuState.buttons[BUTTON_BACK]);
}

void drawCreditsMenu(void) {
  // TODO: Add credits
  drawButton(g_menuState.buttons[BUTTON_BACK]);
}

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
