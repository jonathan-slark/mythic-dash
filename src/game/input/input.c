#include "input.h"
#include <raylib.h>
#include "engine/engine.h"

// --- Constants ---

static const int KEYS[INPUT_KEY_COUNT] = {
  KEY_ESCAPE, KEY_SPACE, KEY_S, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT,  KEY_ENTER, KEY_KP_ENTER,
  KEY_F,      KEY_M,     KEY_P, KEY_C,  KEY_I,     KEY_N,    KEY_MINUS, KEY_EQUAL
};

static const int MOUSE_BUTTONS[INPUT_BUTTON_COUNT] = { MOUSE_BUTTON_LEFT };

// --- Global state ---

static struct {
  bool isKeyPressed[INPUT_KEY_COUNT];
  bool isKeyPressedRepeat[INPUT_KEY_COUNT];
  bool isMouseButtonPressed[INPUT_BUTTON_COUNT];
} g_input = {};

// --- Input functions ---

void input_update(void) {
  for (int i = 0; i < INPUT_KEY_COUNT; i++) {
    if (engine_isKeyPressed(KEYS[i])) g_input.isKeyPressed[i] = true;
    if (engine_isKeyPressedRepeat(KEYS[i])) g_input.isKeyPressedRepeat[i] = true;
  }

  for (int i = 0; i < INPUT_BUTTON_COUNT; i++) {
    if (engine_isMouseButtonPressed(MOUSE_BUTTONS[i])) g_input.isMouseButtonPressed[i] = true;
  }
}

void input_flush(void) {
  for (int i = 0; i < INPUT_KEY_COUNT; i++) {
    g_input.isKeyPressed[i]       = false;
    g_input.isKeyPressedRepeat[i] = false;
  }

  for (int i = 0; i < INPUT_BUTTON_COUNT; i++) {
    g_input.isMouseButtonPressed[i] = false;
  }
}

bool input_isKeyPressed(input_Key key) { return g_input.isKeyPressed[key]; }

bool input_isKeyPressedRepeat(input_Key key) { return g_input.isKeyPressedRepeat[key]; }

bool input_isMouseButtonPressed(input_MouseButton button) { return g_input.isMouseButtonPressed[button]; }

bool input_isMouseButtonClick(input_MouseButton button, Rectangle rectangle) {
  return input_isMouseButtonPressed(button) && engine_isMouseHover(rectangle);
}
