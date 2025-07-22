// clang-format Language: C
#pragma once

// --- Types ---

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
  INPUT_COUNT
} input_Key;

// --- Input functions ---

void input_update(void);
bool input_isKeyPressed(input_Key key);
