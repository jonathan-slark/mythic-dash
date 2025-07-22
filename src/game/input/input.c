#include "input.h"
#include <raylib.h>
#include "engine/engine.h"

// --- Constants ---

static const int KEYS[INPUT_COUNT] = {
  KEY_ESCAPE, KEY_SPACE, KEY_S, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_LEFT,  KEY_ENTER, KEY_KP_ENTER,
  KEY_F,      KEY_M,     KEY_P, KEY_C,  KEY_I,     KEY_N,    KEY_MINUS, KEY_EQUAL
};

// --- Global state ---

static bool g_isKeyPressed[INPUT_COUNT] = {};

// --- Input functions ---

void input_update(void) {
  for (int i = 0; i < INPUT_COUNT; i++) {
    if (engine_isKeyPressed(KEYS[i])) g_isKeyPressed[i] = true;
  }
}

bool input_isKeyPressed(input_Key key) {
  bool isPressed      = g_isKeyPressed[key];
  g_isKeyPressed[key] = false;
  return isPressed;
}
