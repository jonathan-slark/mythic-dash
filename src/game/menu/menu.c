#include "menu.h"
#include <assert.h>
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include <stdio.h>
#include "../draw/draw.h"
#include "../input/input.h"
#include "../internal.h"
#include "../player/player.h"

// --- Types ---

typedef enum {
  MENU_MAIN,
  MENU_GAME,
  MENU_HISCORES,
  MENU_OPTIONS,
  MENU_CREDITS,
  MENU_COUNT,
  MENU_NONE
} menu_ScreenState;

typedef struct menu_Button menu_Button;
typedef struct menu_Button {
  const Rectangle        bounds;
  const char*            text;
  const menu_ScreenState targetScreen;           // Screen to navigate to
  void                   (*const action)(void);  // Custom action callback
  const menu_Context     context;
  const bool             isDropdown;
  const menu_Button*     dropdownItems;
  const int              dropdownItemCount;
  bool                   isExpanded;
} menu_Button;

typedef struct menu_Screen {
  const menu_Button* buttons;
  const int          buttonCount;
  const Rectangle    background;
  const Color        backgroundColour;
  const Color        borderColour;
  void               (*const customDraw)(void);  // Optional custom drawing
} menu_Screen;

typedef struct menu_State {
  menu_ScreenState currentScreen;
  menu_Context     context;
  int              selectedButton[MENU_COUNT];
  int              activatedButton;
  bool             isMouseActive;
  Vector2          lastMousePos;
  int              activeDropdown;
  int              dropdownSelection;
} menu_State;

// --- Function prototypes ---

static void returnToTitle(void);

// --- Constants ---

static const Rectangle BG_MAIN     = { 189, 60, 102, 100 };
static const Rectangle BG_HISCORES = { 129, 60, 222, 150 };
static const Color     BG_COLOUR   = { 64, 64, 64, 200 };
static const Color     BG_BORDER   = { 255, 255, 255, 200 };

static const Color TEXT_NORMAL = { 255, 255, 255, 255 };
static const Color TEXT_ACTIVE = { 151, 255, 49, 255 };

static const char DROP_DOWN_TEXT[] = "[%s: %s \x85]";
static const int  BUFFER_LEN       = 256;

// clang-format off
static const menu_Button MAIN_BUTTONS[] = {
  {  { 195, 70, 60, 10 },      "Start Game",     MENU_GAME,             nullptr,   MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  {  { 195, 80, 66, 10 },     "High Scores", MENU_HISCORES,             nullptr,   MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  {  { 195, 90, 42, 10 },         "Options",  MENU_OPTIONS,             nullptr,   MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  { { 195, 100, 42, 10 },         "Credits",  MENU_CREDITS,             nullptr,   MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  { { 195, 140, 54, 10 },       "Quit Game",     MENU_NONE, engine_requestClose,  MENU_CONTEXT_TITLE, false, nullptr, 0, false },
  { { 195, 120, 66, 10 },     "Resume Game",     MENU_NONE,          menu_close, MENU_CONTEXT_INGAME, false, nullptr, 0, false },
  { { 195, 130, 90, 10 }, "Return to Title",     MENU_NONE,       returnToTitle, MENU_CONTEXT_INGAME, false, nullptr, 0, false },
  { { 195, 140, 90, 10 }, "Exit to Desktop",     MENU_NONE, engine_requestClose, MENU_CONTEXT_INGAME, false, nullptr, 0, false }
};

static const menu_Button SCORES_MODE[] = {
  { { 135, 70, 24, 10 },   "Easy", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  { { 135, 70, 24, 10 }, "Normal", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  { { 135, 70, 24, 10 }, "Arcade", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, false, nullptr, 0, false },
};
static const menu_Button SCORES_SORT[] = {
  { { 237, 70, 24, 10 },   "Time", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  { { 237, 70, 24, 10 },  "Score", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, false, nullptr, 0, false },
};
static const menu_Button SCORES_BUTTONS[] = {
  { { 135,  70,  96, 10 },    "Mode", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, true, SCORES_MODE, COUNT(SCORES_MODE), false },
  { { 237,  70, 108, 10 }, "Sort by", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, true, SCORES_SORT, COUNT(SCORES_SORT), false },
  { { 135, 190,  24, 10 },    "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH, false,    nullptr,                  0, false }
};

static const menu_Button GAME_BUTTONS[] = {
  {  { 195, 70, 24, 10 },        "Easy", MENU_NONE,   game_newEasy, MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  {  { 195, 80, 36, 10 },      "Normal", MENU_NONE, game_newNormal, MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  {  { 195, 90, 66, 10 }, "Arcade Mode", MENU_NONE, game_newArcade, MENU_CONTEXT_BOTH, false, nullptr, 0, false },
  { { 195, 140, 24, 10 },        "Back", MENU_MAIN,        nullptr, MENU_CONTEXT_BOTH, false, nullptr, 0, false }
};

static const menu_Button OPTIONS_BUTTONS[] = {
  { { 195, 140, 24, 10 }, "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH, false, nullptr, 0, false }
};

static const menu_Button CREDITS_BUTTONS[] = {
  { { 195, 140, 24, 10 }, "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH, false, nullptr, 0, false }
};

static const menu_Screen SCREENS[] = {
  [MENU_MAIN]     = {    MAIN_BUTTONS,    COUNT(MAIN_BUTTONS),     BG_MAIN, BG_COLOUR, BG_BORDER,         nullptr },
  [MENU_HISCORES] = {  SCORES_BUTTONS,  COUNT(SCORES_BUTTONS), BG_HISCORES, BG_COLOUR, BG_BORDER, scores_drawMenu },
  [MENU_GAME]     = {    GAME_BUTTONS,    COUNT(GAME_BUTTONS),     BG_MAIN, BG_COLOUR, BG_BORDER,         nullptr },
  [MENU_OPTIONS]  = { OPTIONS_BUTTONS, COUNT(OPTIONS_BUTTONS),     BG_MAIN, BG_COLOUR, BG_BORDER,         nullptr },
  [MENU_CREDITS]  = { CREDITS_BUTTONS, COUNT(CREDITS_BUTTONS),     BG_MAIN, BG_COLOUR, BG_BORDER,         nullptr },
};
// clang-format on

static const float MOUSE_ACTIVE_DISTANCE = 100.0f;

// --- Global state ---

static menu_State g_state = {
  .currentScreen   = MENU_MAIN,
  .context         = MENU_CONTEXT_TITLE,
  .selectedButton  = {},
  .activatedButton = -1,
  .isMouseActive   = false
};

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
      if (g_state.activatedButton == i || input_isMouseButtonClick(INPUT_LEFT_BUTTON, button->bounds)) {
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

  engine_drawRectangle(screen->background, screen->backgroundColour);
  engine_drawRectangleOutline(screen->background, screen->borderColour);

  // First pass: draw regular buttons
  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];
    assert(button != nullptr);
    if (isButtonActive(button)) {
      bool isSelected = !g_state.isMouseActive && g_state.selectedButton[g_state.currentScreen] == i;
      bool isHovered  = g_state.isMouseActive && engine_isMouseHover(button->bounds);

      draw_Text text = {
        .xPos     = button->bounds.x,
        .yPos     = button->bounds.y,
        .colour   = isHovered || isSelected ? TEXT_ACTIVE : TEXT_NORMAL,
        .fontSize = FONT_NORMAL
      };
      char format[BUFFER_LEN];
      if (button->isDropdown) {
        snprintf(format, sizeof(format), DROP_DOWN_TEXT, button->text, button->dropdownItems[0].text);
        text.format = format;
      } else {
        text.format = button->text;
      }
      draw_shadowText(text);
    }
  }

  // Second pass: Draw dropdown containers (on top of buttons)
  for (int i = 0; i < screen->buttonCount; i++) {
  }

  if (screen->customDraw != nullptr) {
    screen->customDraw();
  }
}

static void checkKeys(const menu_Screen* screen) {
  // Note: escape key is handled by game.c
  int button = g_state.selectedButton[g_state.currentScreen];
  if (input_isKeyPressed(INPUT_UP)) {
    g_state.selectedButton[g_state.currentScreen] = findPreviousActiveButton(screen, button);
    g_state.isMouseActive                         = false;
  }
  if (input_isKeyPressed(INPUT_DOWN)) {
    g_state.selectedButton[g_state.currentScreen] = findNextActiveButton(screen, button);
    g_state.isMouseActive                         = false;
  }
  if (input_isKeyPressed(INPUT_ENTER) || input_isKeyPressed(INPUT_KP_ENTER)) {
    g_state.activatedButton = button;
    g_state.isMouseActive   = false;
  }
}

static void checkMouse(void) {
  Vector2 pos = engine_getMousePosition();
  if (Vector2Distance(g_state.lastMousePos, pos) >= MOUSE_ACTIVE_DISTANCE) {
    g_state.isMouseActive = true;
    g_state.lastMousePos  = pos;
  }
}

// --- Menu functions ---

void menu_open(menu_Context context) {
  if (g_game.state == GAME_RUN) player_onPause();

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
  g_state.lastMousePos      = engine_getMousePosition();
  const menu_Screen* screen = &SCREENS[g_state.currentScreen];
  resetButtonState(screen);
}

void menu_close(void) {
  assert(
      g_game.lastState == GAME_PAUSE || g_game.lastState == GAME_START || g_game.lastState == GAME_DEAD ||
      g_game.lastState == GAME_RUN || g_game.lastState == GAME_OVER || g_game.lastState == GAME_LEVELCLEAR
  );
  if (g_state.context == MENU_CONTEXT_INGAME) g_game.state = g_game.lastState;

  if (g_game.state == GAME_RUN) player_onResume();
}

void menu_update(void) {
  const menu_Screen* screen = &SCREENS[g_state.currentScreen];
  updateMenuScreen(screen);
  checkKeys(screen);
  checkMouse();
}

void menu_draw(void) {
  const menu_Screen* screen = &SCREENS[g_state.currentScreen];
  drawMenuScreen(screen);
  if (g_state.isMouseActive) draw_cursor();
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
    case MENU_HISCORES:
    case MENU_OPTIONS:
    case MENU_CREDITS: g_state.currentScreen = MENU_MAIN; break;

    case MENU_COUNT:
    case MENU_NONE: assert(false); break;
  }
}
