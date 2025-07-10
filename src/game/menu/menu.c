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
  menu_Context     context;
} menu_Button;

typedef struct {
  const menu_Button* buttons;
  int                buttonCount;
  void               (*customDraw)(void);  // Optional custom drawing
} menu_Screen;

typedef struct {
  menu_ScreenState currentScreen;
  menu_Context     context;
} menu_State;

// --- Constants ---

static const Color TEXT_NORMAL = { 180, 180, 180, 255 };
static const Color TEXT_ACTIVE = { 255, 255, 255, 255 };

static const menu_Button MAIN_BUTTONS[] = {
  {  { 210, 70, 100, 10 },      "Start Game",    MENU_GAME,             nullptr,   MENU_CONTEXT_BOTH },
  {  { 210, 80, 100, 10 },         "Options", MENU_OPTIONS,             nullptr,   MENU_CONTEXT_BOTH },
  {  { 210, 90, 100, 10 },         "Credits", MENU_CREDITS,             nullptr,   MENU_CONTEXT_BOTH },
  { { 210, 110, 100, 10 },       "Quit Game",    MENU_NONE, engine_requestClose,  MENU_CONTEXT_TITLE },
  { { 210, 110, 100, 10 },     "Resume Game",    MENU_NONE,          menu_close, MENU_CONTEXT_INGAME },
  { { 210, 120, 100, 10 }, "Return to Title",    MENU_NONE,             nullptr, MENU_CONTEXT_INGAME },
  { { 210, 130, 100, 10 }, "Exit to Desktop",    MENU_NONE, engine_requestClose, MENU_CONTEXT_INGAME },
};
static const menu_Button GAME_BUTTONS[] = {
  {  { 210, 70, 100, 10 },   "Easy", MENU_NONE, game_new, MENU_CONTEXT_BOTH }, // TODO: add difficulty parameter
  {  { 210, 80, 100, 10 }, "Normal", MENU_NONE, game_new, MENU_CONTEXT_BOTH },
  {  { 210, 90, 100, 10 }, "Arcade", MENU_NONE, game_new, MENU_CONTEXT_BOTH },
  { { 210, 110, 100, 10 },   "Back", MENU_MAIN,  nullptr, MENU_CONTEXT_BOTH }
};
static const menu_Button OPTIONS_BUTTONS[] = {
  { { 210, 110, 100, 10 }, "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH }
};
static const menu_Button CREDITS_BUTTONS[] = {
  { { 210, 110, 100, 10 }, "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH }
};

static const menu_Screen SCREENS[] = {
  [MENU_MAIN]    = {    MAIN_BUTTONS,    COUNT(MAIN_BUTTONS), nullptr },
  [MENU_GAME]    = {    GAME_BUTTONS,    COUNT(GAME_BUTTONS), nullptr },
  [MENU_OPTIONS] = { OPTIONS_BUTTONS, COUNT(OPTIONS_BUTTONS), nullptr },
  [MENU_CREDITS] = { CREDITS_BUTTONS, COUNT(CREDITS_BUTTONS), nullptr },
};

// --- Global state ---

static menu_State g_state = { MENU_MAIN, MENU_CONTEXT_TITLE };

// --- Helper functions ---

void updateMenuScreen(const menu_Screen* screen) {
  assert(screen != nullptr);

  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];
    assert(button != nullptr);
    if (button->context == MENU_CONTEXT_BOTH ||
        (button->context == MENU_CONTEXT_TITLE && g_state.context == MENU_CONTEXT_TITLE) ||
        (button->context == MENU_CONTEXT_INGAME && g_state.context == MENU_CONTEXT_INGAME)) {
      if (engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, button->bounds)) {
        if (button->action != nullptr) {
          button->action();
        } else if (button->targetScreen != MENU_NONE) {
          g_state.currentScreen = button->targetScreen;
        }
      }
    }
  }
}

void drawMenuScreen(const menu_Screen* screen) {
  assert(screen != nullptr);

  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];

    if (button->context == MENU_CONTEXT_BOTH ||
        (button->context == MENU_CONTEXT_TITLE && g_state.context == MENU_CONTEXT_TITLE) ||
        (button->context == MENU_CONTEXT_INGAME && g_state.context == MENU_CONTEXT_INGAME)) {
      bool isHovered = engine_isMouseHover(button->bounds);
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
  }

  if (screen->customDraw != nullptr) {
    screen->customDraw();
  }
}

// --- Menu functions ---

void menu_open(menu_Context context) {
  if (context == MENU_CONTEXT_INGAME) {
    g_game.lastState = g_game.state;
    g_game.state     = GAME_MENU;
  }
  g_state.currentScreen = MENU_MAIN;
  g_state.context       = context;
  engine_showCursor();
}

void menu_close(void) {
  assert(
      g_game.lastState == GAME_PAUSE || g_game.lastState == GAME_READY || g_game.lastState == GAME_RUN ||
      g_game.lastState == GAME_OVER
  );
  if (g_state.context == MENU_CONTEXT_INGAME) g_game.state = g_game.lastState;
  engine_hideCursor();
}

void menu_update(void) {
  const menu_Screen* current = &SCREENS[g_state.currentScreen];
  updateMenuScreen(current);
}

void menu_draw(void) {
  const menu_Screen* current = &SCREENS[g_state.currentScreen];
  drawMenuScreen(current);
}

void menu_back(void) {
  switch (g_state.currentScreen) {
    case MENU_MAIN: engine_requestClose(); break;
    case MENU_GAME:
    case MENU_OPTIONS:
    case MENU_CREDITS: g_state.currentScreen = MENU_MAIN; break;
    case MENU_NONE: assert(false); break;
  }
}
