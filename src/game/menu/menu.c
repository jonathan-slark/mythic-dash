#include "menu.h"
#include <assert.h>
#include <engine/engine.h>
#include <log/log.h>
#include <raylib.h>
#include <stdio.h>
#include "../audio/audio.h"
#include "../draw/draw.h"
#include "../input/input.h"
#include "../internal.h"
#include "../player/player.h"
#include "../scores/scores.h"

// --- Types ---

typedef enum menu_ScreenState {
  MENU_MAIN,
  MENU_GAME,
  MENU_RESUME,
  MENU_HISCORES,
  MENU_OPTIONS,
  MENU_CREDITS,
  MENU_COUNT,
  MENU_NONE
} menu_ScreenState;

typedef enum menu_ButtonType {
  MENU_BUTTON_NORMAL,
  MENU_BUTTON_DROPDOWN,
  MENU_BUTTON_SLIDER,
  MENU_BUTTON_TEXT
} menu_ButtonType;

typedef struct menu_Button menu_Button;
typedef struct menu_Button {
  Rectangle        bounds;
  const char*      text;
  menu_ScreenState targetScreen;     // Screen to navigate to
  void             (*action)(void);  // Custom action callback
  menu_Context     context;
  menu_ButtonType  type;

  // Dropdown
  menu_Button* dropdownItems;
  int          dropdownItemCount;
  int          selectedItem;

  // Slider
  float* sliderValue;
  float  sliderMin;
  float  sliderMax;
} menu_Button;

typedef struct menu_Screen {
  menu_Button* buttons;
  int          buttonCount;
  Rectangle    background;
  Color        backgroundColour;
  Color        borderColour;
  void         (*customDraw)(void);  // Optional custom drawing
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

static const Rectangle BG_MAIN             = { 159, 60, 162, 100 };
static const Rectangle BG_HISCORES         = { 132, 60, 216, 150 };
static const Color     BG_COLOUR           = { 64, 64, 64, 200 };
static const Color     BG_DROP_DOWN_COLOUR = { 40, 40, 40, 200 };
static const Color     BG_BORDER           = { 255, 255, 255, 200 };

static const Color TEXT_NORMAL = { 255, 255, 255, 255 };
static const Color TEXT_ACTIVE = { 151, 255, 49, 255 };

static const char DROP_DOWN_TEXT[] = "[%s: %s \x85]";
static const int  BUFFER_LEN       = 256;
static const int  TEXT_WIDTH       = 6;

static const Vector2 SLIDER_SIZE   = { 10 * TEXT_WIDTH, 7 };
static const Color   SLIDER_COLOUR = { 239, 177, 0, 200 };

// clang-format off
static menu_Button MAIN_BUTTONS[] = {
  {  { 165, 70, 60, 10 },      "Start Game",     MENU_GAME,             nullptr,   MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  {  { 165, 80, 66, 10 },     "High Scores", MENU_HISCORES,             nullptr,   MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  {  { 165, 90, 42, 10 },         "Options",  MENU_OPTIONS,             nullptr,   MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 100, 42, 10 },         "Credits",  MENU_CREDITS,             nullptr,   MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 140, 54, 10 },       "Quit Game",     MENU_NONE, engine_requestClose,  MENU_CONTEXT_TITLE, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 120, 66, 10 },     "Resume Game",     MENU_NONE,          menu_close, MENU_CONTEXT_INGAME, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 130, 90, 10 }, "Return to Title",     MENU_NONE,       returnToTitle, MENU_CONTEXT_INGAME, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 140, 90, 10 }, "Exit to Desktop",     MENU_NONE, engine_requestClose, MENU_CONTEXT_INGAME, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 }
};

static menu_Button SCORES_MODE[] = {
  { { 138, 70, 24, 10 }, "Easy  ", MENU_NONE, scores_setEasy,   MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 138, 70, 24, 10 }, "Normal", MENU_NONE, scores_setNormal, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 138, 70, 24, 10 }, "Arcade", MENU_NONE, scores_setArcade, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
};
static menu_Button SCORES_SORT[] = {
  { { 234, 70, 24, 10 },  "Time ", MENU_NONE, scores_setTime,  MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 234, 70, 24, 10 },  "Score", MENU_NONE, scores_setScore, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
};
static menu_Button SCORES_BUTTONS[] = {
  { { 138,  70,  96, 10 },    "Mode", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, MENU_BUTTON_DROPDOWN, SCORES_MODE, COUNT(SCORES_MODE), 0, nullptr, 0, 0 },
  { { 234,  70, 108, 10 }, "Sort by", MENU_NONE, nullptr, MENU_CONTEXT_BOTH, MENU_BUTTON_DROPDOWN, SCORES_SORT, COUNT(SCORES_SORT), 0, nullptr, 0, 0 },
  { { 138, 190,  24, 10 },    "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH,   MENU_BUTTON_NORMAL,     nullptr,                  0, 0, nullptr, 0, 0 }
};

static menu_Button GAME_DIFFICULTY[] = {
  { { 165, 70, 24, 10 }, "Easy   ", MENU_NONE,   game_setEasy, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 70, 24, 10 }, "Normal ", MENU_NONE, game_setNormal, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 70, 24, 10 }, "Arcade ", MENU_NONE, game_setArcade, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
};
static menu_Button GAME_LEVEL[] = {
  { { 165, 70, 24, 10 },   "Level 1", MENU_NONE, game_setLevel1, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 70, 24, 10 },   "Level 2", MENU_NONE, game_setLevel2, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 70, 24, 10 },   "Level 3", MENU_NONE, game_setLevel3, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 70, 24, 10 },   "Level 4", MENU_NONE, game_setLevel4, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 70, 24, 10 },   "Level 5", MENU_NONE, game_setLevel5, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 70, 24, 10 },   "Level 6", MENU_NONE, game_setLevel6, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
  { { 165, 70, 24, 10 },   "Level 7", MENU_NONE, game_setLevel7, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 },
};
static menu_Button GAME_BUTTONS[] = {
  {  { 165,  70,  60, 10 }, "Start Game", MENU_NONE, game_start, MENU_CONTEXT_BOTH,   MENU_BUTTON_NORMAL,         nullptr,                      0, 0, nullptr, 0, 0 },
  {  { 165,  90, 138, 10 }, "Difficulty", MENU_NONE,    nullptr, MENU_CONTEXT_BOTH, MENU_BUTTON_DROPDOWN, GAME_DIFFICULTY, COUNT(GAME_DIFFICULTY), 0, nullptr, 0, 0 },
  {  { 165, 100, 138, 10 }, "Level     ", MENU_NONE,    nullptr, MENU_CONTEXT_BOTH, MENU_BUTTON_DROPDOWN,      GAME_LEVEL,      COUNT(GAME_LEVEL), 0, nullptr, 0, 0 },
  {  { 165, 140,  24, 10 },       "Back", MENU_MAIN,    nullptr, MENU_CONTEXT_BOTH,   MENU_BUTTON_NORMAL,         nullptr,                      0, 0, nullptr, 0, 0 }
};

static menu_Button OPTIONS_BUTTONS[] = {
  { { 165,  70, 36, 10 },         "Volume", MENU_MAIN,              nullptr, MENU_CONTEXT_BOTH,   MENU_BUTTON_TEXT, nullptr, 0, 0,          nullptr, 0, 0 },
  { { 165,  80, 84, 10 }, " Master       ", MENU_MAIN, audio_onVolumeChange, MENU_CONTEXT_BOTH, MENU_BUTTON_SLIDER, nullptr, 0, 0, &g_volume.master, 0, 1 },
  { { 165,  90, 84, 10 }, " Music        ", MENU_MAIN, audio_onVolumeChange, MENU_CONTEXT_BOTH, MENU_BUTTON_SLIDER, nullptr, 0, 0,  &g_volume.music, 0, 1 },
  { { 165, 100, 84, 10 }, " Sound Effects", MENU_MAIN, audio_onVolumeChange, MENU_CONTEXT_BOTH, MENU_BUTTON_SLIDER, nullptr, 0, 0,    &g_volume.sfx, 0, 1 },
  { { 165, 140, 24, 10 },          "Back", MENU_MAIN,              nullptr, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0,          nullptr, 0, 0 }
};

static menu_Button CREDITS_BUTTONS[] = {
  { { 165, 140, 24, 10 }, "Back", MENU_MAIN, nullptr, MENU_CONTEXT_BOTH, MENU_BUTTON_NORMAL, nullptr, 0, 0, nullptr, 0, 0 }
};

static menu_Screen SCREENS[] = {
  [MENU_MAIN]     = {    MAIN_BUTTONS,    COUNT(MAIN_BUTTONS),     BG_MAIN, BG_COLOUR, BG_BORDER,             nullptr },
  [MENU_HISCORES] = {  SCORES_BUTTONS,  COUNT(SCORES_BUTTONS), BG_HISCORES, BG_COLOUR, BG_BORDER,     scores_drawMenu },
  [MENU_GAME]     = {    GAME_BUTTONS,    COUNT(GAME_BUTTONS),     BG_MAIN, BG_COLOUR, BG_BORDER, player_drawContinue },
  [MENU_OPTIONS]  = { OPTIONS_BUTTONS, COUNT(OPTIONS_BUTTONS),     BG_MAIN, BG_COLOUR, BG_BORDER,             nullptr },
  [MENU_CREDITS]  = { CREDITS_BUTTONS, COUNT(CREDITS_BUTTONS),     BG_MAIN, BG_COLOUR, BG_BORDER,             nullptr }
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

// New helper functions for dropdown handling
static void handleDropdownInput(const menu_Screen* screen) {
  menu_Button* button = &screen->buttons[g_state.activeDropdown];
  for (int i = 0; i < button->dropdownItemCount; i++) {
    const menu_Button* item         = &button->dropdownItems[i];
    Rectangle          dropdownRect = {
      button->bounds.x,
      button->bounds.y + button->bounds.height,
      button->bounds.width,
      button->bounds.height * (button->dropdownItemCount + 2)
    };
    Rectangle itemRect = {
      dropdownRect.x, dropdownRect.y + (i + 1) * button->bounds.height, dropdownRect.width, button->bounds.height
    };
    if (g_state.activatedButton == i || input_isMouseButtonClick(INPUT_LEFT_BUTTON, itemRect)) {
      if (item->action != nullptr) {
        item->action();
      }
      button->selectedItem    = i;
      g_state.activeDropdown  = -1;
      g_state.activatedButton = -1;
    }
  }
}

static void drawDropdown(const menu_Button* button, int index) {
  if (g_state.activeDropdown == index && button->dropdownItems) {
    // Create dropdown container below button
    Rectangle dropdownRect = {
      button->bounds.x,
      button->bounds.y + button->bounds.height,
      button->bounds.width,
      button->bounds.height * (button->dropdownItemCount + 2)
    };

    // Draw dropdown background
    engine_drawRectangle(dropdownRect, BG_DROP_DOWN_COLOUR);
    engine_drawRectangleOutline(dropdownRect, BG_BORDER);

    // Draw dropdown items
    for (int j = 0; j < button->dropdownItemCount; j++) {
      const menu_Button* item = &button->dropdownItems[j];
      if (isButtonActive(item)) {
        Rectangle itemRect = {
          dropdownRect.x, dropdownRect.y + (j + 1) * button->bounds.height, dropdownRect.width, button->bounds.height
        };

        bool isSelected    = !g_state.isMouseActive && g_state.dropdownSelection == j;
        bool isItemHovered = g_state.isMouseActive && engine_isMouseHover(itemRect);

        draw_Text itemText = {
          .xPos     = itemRect.x + TEXT_WIDTH,
          .yPos     = itemRect.y,
          .format   = item->text,
          .colour   = isItemHovered || isSelected ? TEXT_ACTIVE : TEXT_NORMAL,
          .fontSize = FONT_NORMAL
        };
        draw_shadowText(itemText);
      }
    }
  }
}

static void updateMenuScreen(const menu_Screen* screen) {
  assert(screen != nullptr);

  if (g_state.activeDropdown != -1) {
    handleDropdownInput(screen);
  }

  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];
    assert(button != nullptr);
    if (isButtonActive(button)) {
      if (g_state.activatedButton == i || input_isMouseButtonClick(INPUT_LEFT_BUTTON, button->bounds)) {
        switch (button->type) {
          case MENU_BUTTON_NORMAL:
            if (button->action != nullptr) {
              button->action();
            }
            if (button->targetScreen != MENU_NONE) {
              g_state.currentScreen = button->targetScreen;
              screen                = &SCREENS[g_state.currentScreen];
              resetButtonState(screen);
              break;
            }
            break;

          case MENU_BUTTON_DROPDOWN:
            g_state.dropdownSelection = button->selectedItem;
            if (g_state.activeDropdown != i) {
              g_state.activeDropdown = i;
            } else {
              g_state.activeDropdown = -1;
            }
            break;

          case MENU_BUTTON_TEXT:
          case MENU_BUTTON_SLIDER: break;
        }
      }
    }
  }
}

static void drawButtonText(const menu_Button* button, const char* string, Color colour) {
  draw_Text text = {
    .format = string, .xPos = button->bounds.x, .yPos = button->bounds.y, .colour = colour, .fontSize = FONT_NORMAL
  };
  draw_shadowText(text);
}

static void drawNormalButton(const menu_Button* button, bool isSelected, bool isHovered) {
  Color colour = isHovered || isSelected ? TEXT_ACTIVE : TEXT_NORMAL;
  drawButtonText(button, button->text, colour);
}

static void drawDropdownButton(const menu_Button* button, bool isSelected, bool isHovered) {
  Color colour = isHovered || isSelected ? TEXT_ACTIVE : TEXT_NORMAL;
  char  format[BUFFER_LEN];
  snprintf(format, sizeof(format), DROP_DOWN_TEXT, button->text, button->dropdownItems[button->selectedItem].text);
  drawButtonText(button, format, colour);
}

static void drawSliderButton(const menu_Button* button, bool isSelected) {
  Color colour = isSelected ? TEXT_ACTIVE : TEXT_NORMAL;
  drawButtonText(button, button->text, colour);

  Rectangle outline = {
    button->bounds.x + button->bounds.width + TEXT_WIDTH, button->bounds.y, SLIDER_SIZE.x, SLIDER_SIZE.y
  };
  engine_drawRectangleOutline(outline, BG_BORDER);

  Rectangle slider = {
    button->bounds.x + button->bounds.width + TEXT_WIDTH + 2, button->bounds.y + 2, SLIDER_SIZE.x - 4, SLIDER_SIZE.y - 4
  };
  engine_drawRectangle(slider, SLIDER_COLOUR);
}

static void drawTextButton(const menu_Button* button) { drawButtonText(button, button->text, TEXT_NORMAL); }

static void drawMenuScreen(const menu_Screen* screen) {
  assert(screen != nullptr);

  engine_drawRectangle(screen->background, screen->backgroundColour);
  engine_drawRectangleOutline(screen->background, screen->borderColour);

  // Draw regular buttons
  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];
    assert(button != nullptr);
    if (isButtonActive(button)) {
      bool isSelected = !g_state.isMouseActive && g_state.selectedButton[g_state.currentScreen] == i;
      bool isHovered  = g_state.isMouseActive && engine_isMouseHover(button->bounds);

      switch (button->type) {
        case MENU_BUTTON_NORMAL: drawNormalButton(button, isSelected, isHovered); break;
        case MENU_BUTTON_DROPDOWN: drawDropdownButton(button, isSelected, isHovered); break;
        case MENU_BUTTON_SLIDER: drawSliderButton(button, isSelected); break;
        case MENU_BUTTON_TEXT: drawTextButton(button); break;
      }
    }
  }

  if (screen->customDraw != nullptr) {
    screen->customDraw();
  }

  // Draw dropdowns if any
  for (int i = 0; i < screen->buttonCount; i++) {
    const menu_Button* button = &screen->buttons[i];
    if (isButtonActive(button) && button->type == MENU_BUTTON_DROPDOWN) {
      drawDropdown(button, i);
    }
  }
}

static void checkKeys(const menu_Screen* screen) {
  // Handle dropdown navigation if active
  if (g_state.activeDropdown != -1) {
    menu_Button* dropdownBtn = &screen->buttons[g_state.activeDropdown];

    // Navigate dropdown items
    if (input_isKeyPressed(INPUT_UP)) {
      g_state.dropdownSelection =
          (g_state.dropdownSelection - 1 + dropdownBtn->dropdownItemCount) % dropdownBtn->dropdownItemCount;
      g_state.isMouseActive = false;
    }
    if (input_isKeyPressed(INPUT_DOWN)) {
      g_state.dropdownSelection = (g_state.dropdownSelection + 1) % dropdownBtn->dropdownItemCount;
      g_state.isMouseActive     = false;
    }
    if (input_isKeyPressed(INPUT_ENTER) || input_isKeyPressed(INPUT_KP_ENTER)) {
      g_state.activatedButton   = g_state.dropdownSelection;
      dropdownBtn->selectedItem = g_state.dropdownSelection;
      g_state.isMouseActive     = false;
    }

    return;  // Skip main navigation when dropdown is active
  }

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
    // Toggle dropdown if button has one
    if (screen->buttons[button].type == MENU_BUTTON_DROPDOWN) {
      g_state.activeDropdown = button;
    }
    // Otherwise activate normally
    else {
      g_state.activatedButton = button;
    }
    g_state.isMouseActive = false;
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
  g_state.activeDropdown    = -1;
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
      if (g_state.activeDropdown != -1) {
        g_state.activeDropdown = -1;
      } else if (g_state.context == MENU_CONTEXT_TITLE) {
        engine_requestClose();
      } else {
        menu_close();
      }
      break;

    case MENU_RESUME:
      if (g_state.activeDropdown != -1) {
        g_state.activeDropdown = -1;
      } else {
        g_state.currentScreen = MENU_GAME;
      }
      break;

    case MENU_GAME:
    case MENU_HISCORES:
    case MENU_OPTIONS:
    case MENU_CREDITS:
      if (g_state.activeDropdown != -1) {
        g_state.activeDropdown = -1;
      } else {
        g_state.currentScreen = MENU_MAIN;
      }
      break;

    case MENU_COUNT:
    case MENU_NONE: assert(false); break;
  }
}
