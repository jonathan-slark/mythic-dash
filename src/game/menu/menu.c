#include "menu.h"
#include <assert.h>
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include "../draw/draw.h"
#include "../internal.h"

// --- Types ---

typedef enum { MENU_MAIN, MENU_GAME, MENU_OPTIONS, MENU_CREDITS, MENU_NONE } menu_ScreenState;

typedef struct {
  Rectangle        bounds;
  const char*      text;
  menu_ScreenState targetScreen;     // Screen to navigate to
  void             (*action)(void);  // Custom action callback
} menu_Button;

typedef struct {
  const menu_Button* buttons;
  int                buttonCount;
  void               (*customDraw)(void);  // Optional custom drawing
} menu_Screen;

// --- Constants ---

static const Color TEXT_NORMAL = { 200, 200, 200, 255 };
static const Color TEXT_ACTIVE = { 255, 255, 255, 255 };

static const menu_Button MAIN_BUTTONS[] = {
  {  { 200, 70, 100, 10 }, "Start Game",    MENU_GAME,             nullptr },
  {  { 200, 80, 100, 10 },    "Options", MENU_OPTIONS,             nullptr },
  {  { 200, 90, 100, 10 },    "Credits", MENU_CREDITS,             nullptr },
  { { 200, 100, 100, 10 },  "Quit Game",    MENU_NONE, engine_requestClose }
};
static const menu_Button GAME_BUTTONS[] = {
  {  { 200, 70, 100, 10 },   "Easy", MENU_NONE, game_new }, // TODO: add difficulty parameter
  {  { 200, 80, 100, 10 }, "Normal", MENU_NONE, game_new },
  {  { 200, 90, 100, 10 }, "Arcade", MENU_NONE, game_new },
  { { 200, 100, 100, 10 },   "Back", MENU_MAIN,  nullptr }
};
static const menu_Button OPTIONS_BUTTONS[] = {
  { { 200, 100, 100, 10 }, "Back", MENU_MAIN, nullptr }
};
static const menu_Button CREDITS_BUTTONS[] = {
  { { 200, 100, 100, 10 }, "Back", MENU_MAIN, nullptr }
};

static const menu_Screen SCREENS[] = {
  [MENU_MAIN]    = {    MAIN_BUTTONS,    COUNT(MAIN_BUTTONS), nullptr },
  [MENU_GAME]    = {    GAME_BUTTONS,    COUNT(GAME_BUTTONS), nullptr },
  [MENU_OPTIONS] = { OPTIONS_BUTTONS, COUNT(OPTIONS_BUTTONS), nullptr },
  [MENU_CREDITS] = { CREDITS_BUTTONS, COUNT(CREDITS_BUTTONS), nullptr },
};

// --- Global state ---

menu_ScreenState g_currentScreen = MENU_MAIN;

// --- Helper functions ---

void updateMenuScreen(const menu_Screen* screen) {
  assert(screen != nullptr);

  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];
    assert(button != nullptr);

    if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, button->bounds)) {
      if (button->action != nullptr) {
        button->action();
      } else if (button->targetScreen != MENU_NONE) {
        g_currentScreen = button->targetScreen;
      }
    }
  }
}

void drawMenuScreen(const menu_Screen* screen) {
  assert(screen != nullptr);

  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button    = &screen->buttons[i];
    bool               isHovered = engine_isMouseHover(button->bounds);
    assert(button != nullptr);

    draw_Text text = {
      .xPos     = button->bounds.x,
      .yPos     = button->bounds.y,
      .format   = button->text,
      .colour   = isHovered ? TEXT_ACTIVE : TEXT_NORMAL,
      .fontSize = FONT_NORMAL
    };
    draw_text(text);
  }

  if (screen->customDraw != nullptr) {
    screen->customDraw();
  }
}

// --- Menu functions ---

void menu_update(void) {
  const menu_Screen* current = &SCREENS[g_currentScreen];
  updateMenuScreen(current);
}

void menu_draw(void) {
  const menu_Screen* current = &SCREENS[g_currentScreen];
  drawMenuScreen(current);
}
