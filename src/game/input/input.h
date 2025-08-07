// clang-format Language: C
#pragma once

// --- Types ---

#include <raylib.h>
typedef enum input_Key {
  INPUT_ESCAPE,
  INPUT_SPACE,
  INPUT_S,
  INPUT_UP,
  INPUT_RIGHT,
  INPUT_DOWN,
  INPUT_LEFT,
  INPUT_ENTER,
  INPUT_KP_ENTER,
  INPUT_F,
  INPUT_M,
  INPUT_P,
  INPUT_C,
  INPUT_I,
  INPUT_N,
  INPUT_MINUS,
  INPUT_EQUAL,
  INPUT_KEY_COUNT
} input_Key;

typedef enum input_MouseButton { INPUT_LEFT_BUTTON, INPUT_BUTTON_COUNT } input_MouseButton;

// --- Input functions ---

void input_update(void);
void input_flush(void);
bool input_isKeyPressed(input_Key key);
bool input_isKeyPressedRepeat(input_Key key);
bool input_isMouseButtonPressed(input_MouseButton button);
bool input_isMouseButtonClick(input_MouseButton button, Rectangle rectangle);
