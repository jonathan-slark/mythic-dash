#include "menu.h"
#include <assert.h>
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include "../draw/draw.h"
#include "../internal.h"

// --- Types ---

typedef enum { MENU_MAIN, MENU_GAME, MENU_OPTIONS, MENU_CREDITS, MENU_COUNT, MENU_NONE } menu_ScreenState;

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
  int              selectedButton[MENU_COUNT];
  int              activatedButton;
} menu_State;

// --- Function prototypes ---

static void returnToTitle(void);

// --- Constants ---

static const Rectangle BACKGROUND_RECTANGLE = { 180, 60, 110, 90 };
static const Color     BACKGROUND_COLOUR    = { 64, 64, 64, 200 };
static const Color     BACKGROUND_BORDER    = { 255, 255, 255, 200 };

static const Color TEXT_NORMAL = { 180, 180, 180, 255 };
static const Color TEXT_ACTIVE = { 255, 255, 255, 255 };

static const menu_Button MAIN_BUTTONS[] = {
  {  { 190, 70, 100, 10 },      "Start Game",    MENU_GAME,             nullptr,   MENU_CONTEXT_BOTH },
  {  { 190, 80, 100, 10 },         "Options", MENU_OPTIONS,             nullptr,   MENU_CONTEXT_BOTH },
  {  { 190, 90, 100, 10 },         "Credits", MENU_CREDITS,             nullptr,   MENU_CONTEXT_BOTH },
  { { 190, 130, 100, 10 },       "Quit Game",    MENU_NONE, engine_requestClose,  MENU_CONTEXT_TITLE },
  { { 190, 110, 100, 10 },     "Resume Game",    MENU_NONE,          menu_close, MENU_CONTEXT_INGAME },
  { { 190, 120, 100, 10 }, "Return to Title",    MENU_NONE,       returnToTitle, MENU_CONTEXT_INGAME },
  { { 190, 130, 100, 10 }, "Exit to Desktop",    MENU_NONE, engine_requestClose, MENU_CONTEXT_INGAME },
};
static const menu_Button GAME_BUTTONS[] = {
  {  { 190, 70, 100, 10 },   "Easy", MENU_NONE, game_new, MENU_CONTEXT_BOTH }, // TODO: add difficulty parameter
  {  { 190, 80, 100, 10 }, "Normal", MENU_NONE, game_new, MENU_CONTEXT_BOTH },
  {  { 190, 90, 100, 10 }, "Arcade", MENU_NONE, game_new, MENU_CONTEXT_BOTH },
  { { 190, 130, 100, 10 },   "Back", MENU_MAIN,  nullptr, MENU_CONTEXT_BOTH }
};
static const menu_Button OPTIONS_BUTTONS[] = {
  { { 190, 130, 100, 10 }, "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH }
};
static const menu_Button CREDITS_BUTTONS[] = {
  { { 190, 130, 100, 10 }, "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH }
};

static const menu_Screen SCREENS[] = {
  [MENU_MAIN]    = {    MAIN_BUTTONS,    COUNT(MAIN_BUTTONS), nullptr },
  [MENU_GAME]    = {    GAME_BUTTONS,    COUNT(GAME_BUTTONS), nullptr },
  [MENU_OPTIONS] = { OPTIONS_BUTTONS, COUNT(OPTIONS_BUTTONS), nullptr },
  [MENU_CREDITS] = { CREDITS_BUTTONS, COUNT(CREDITS_BUTTONS), nullptr },
};

// --- Global state ---

static menu_State g_state = { MENU_MAIN, MENU_CONTEXT_TITLE, {}, -1 };

// --- Helper functions ---

static void returnToTitle(void) { menu_open(MENU_CONTEXT_TITLE); }

static bool isButtonActive(const menu_Button* button) {
  return button->context == MENU_CONTEXT_BOTH ||
         (button->context == MENU_CONTEXT_TITLE && g_state.context == MENU_CONTEXT_TITLE) ||
         (button->context == MENU_CONTEXT_INGAME && g_state.context == MENU_CONTEXT_INGAME);
}

static int findPreviousActiveButton(const menu_Screen* screen, int start) {
  int index    = start;
  int attempts = 0;
  do {
    index = (index - 1 + screen->buttonCount) % screen->buttonCount;
    attempts++;
  } while (!isButtonActive(&screen->buttons[index]) && attempts < screen->buttonCount);
  return index;
}

static int findNextActiveButton(const menu_Screen* screen, int start) {
  int index    = start;
  int attempts = 0;
  do {
    index = (index + 1) % screen->buttonCount;
    attempts++;
  } while (!isButtonActive(&screen->buttons[index]) && attempts < screen->buttonCount);
  return index;
}

static void resetButtonState(const menu_Screen* screen) {
  int button = g_state.selectedButton[g_state.currentScreen];
  if (!isButtonActive(&screen->buttons[button]))
    g_state.selectedButton[g_state.currentScreen] = findPreviousActiveButton(screen, button);
  g_state.activatedButton = -1;
}

static void updateMenuScreen(const menu_Screen* screen) {
  assert(screen != nullptr);

  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];
    assert(button != nullptr);
    if (isButtonActive(button)) {
      if (g_state.activatedButton == i || engine_isMouseButtonClick(MOUSE_LEFT_BUTTON, button->bounds)) {
        if (button->action != nullptr) {
          button->action();
        } else if (button->targetScreen != MENU_NONE) {
          g_state.currentScreen = button->targetScreen;
          screen                = &SCREENS[g_state.currentScreen];
          resetButtonState(screen);
          break;
        }
      }
    }
  }
}

static void drawMenuScreen(const menu_Screen* screen) {
  assert(screen != nullptr);

  engine_drawRectangle(BACKGROUND_RECTANGLE, BACKGROUND_COLOUR);
  engine_drawRectangleOutline(BACKGROUND_RECTANGLE, BACKGROUND_BORDER);

  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];
    assert(button != nullptr);
    if (isButtonActive(button)) {
      bool isHovered = g_state.selectedButton[g_state.currentScreen] == i || engine_isMouseHover(button->bounds);

      draw_Text text = {
        .xPos     = button->bounds.x,
        .yPos     = button->bounds.y,
        .format   = button->text,
        .colour   = isHovered ? TEXT_ACTIVE : TEXT_NORMAL,
        .fontSize = FONT_NORMAL
      };
      draw_shadowText(text);
    }
  }

  if (screen->customDraw != nullptr) {
    screen->customDraw();
  }
}

static void checkKeys(const menu_Screen* screen) {
  // Note: escape key is handled by game.c
  int button = g_state.selectedButton[g_state.currentScreen];
  if (engine_isKeyPressed(KEY_UP))
    g_state.selectedButton[g_state.currentScreen] = findPreviousActiveButton(screen, button);
  if (engine_isKeyPressed(KEY_DOWN))
    g_state.selectedButton[g_state.currentScreen] = findNextActiveButton(screen, button);
  if (engine_isKeyPressed(KEY_ENTER) || engine_isKeyPressed(KEY_KP_ENTER)) g_state.activatedButton = button;
}

// --- Menu functions ---

void menu_open(menu_Context context) {
  switch (context) {
    case MENU_CONTEXT_TITLE:
      g_game.lastState = GAME_BOOT;
      g_game.state     = GAME_TITLE;
      break;

    case MENU_CONTEXT_INGAME:
      g_game.lastState = g_game.state;
      g_game.state     = GAME_MENU;
      break;

    default: assert(false); break;
  }

  g_state.currentScreen     = MENU_MAIN;
  g_state.context           = context;
  const menu_Screen* screen = &SCREENS[g_state.currentScreen];
  resetButtonState(screen);
}

void menu_close(void) {
  assert(
      g_game.lastState == GAME_PAUSE || g_game.lastState == GAME_READY || g_game.lastState == GAME_RUN ||
      g_game.lastState == GAME_OVER
  );
  if (g_state.context == MENU_CONTEXT_INGAME) g_game.state = g_game.lastState;
}

void menu_update(void) {
  const menu_Screen* screen = &SCREENS[g_state.currentScreen];
  updateMenuScreen(screen);
  checkKeys(screen);
}

void menu_draw(void) {
  const menu_Screen* screen = &SCREENS[g_state.currentScreen];
  drawMenuScreen(screen);
  draw_cursor();
}

void menu_back(void) {
  switch (g_state.currentScreen) {
    case MENU_MAIN:
      if (g_state.context == MENU_CONTEXT_TITLE) {
        engine_requestClose();
      } else {
        menu_close();
      }
      break;

    case MENU_GAME:
    case MENU_OPTIONS:
    case MENU_CREDITS: g_state.currentScreen = MENU_MAIN; break;

    case MENU_COUNT:
    case MENU_NONE: assert(false); break;
  }
}
