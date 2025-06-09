#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "engine.h"
#include "internal.h"

// --- Constants ---

constexpr int BUFFER_SIZE = 32;

// --- Renderer functions ---

void engine_drawBackground(engine_Texture* background) {
  if (background == nullptr) {
    LOG_WARN(engine__log, "Background is nullptr");
    return;
  }

  DrawTextureEx(background->texture, (Vector2) { 0, 0 }, 0, engine__screenState.scale, WHITE);
}

void engine_drawRectangleOutline(Rectangle rect, Color color) {
  int       scale  = engine__screenState.scale;
  Rectangle scaled = (Rectangle) { rect.x * scale, rect.y * scale, rect.width * scale, rect.height * scale };
  DrawRectangleLinesEx(scaled, (float) scale, color);
}

void engine_drawText(const char* text, Vector2 pos, int size, Color colour) {
  int     scale     = engine__screenState.scale;
  Vector2 scaledPos = Vector2Scale(pos, scale);
  DrawText(text, scaledPos.x, scaledPos.y, size, colour);
}

void engine_drawFloat(float number, Vector2 pos, int size, Color colour) {
  if (size <= 0) {
    LOG_ERROR(engine__log, "Invalid size: %d", size);
    return;
  }

  char numberText[BUFFER_SIZE];
  snprintf(numberText, sizeof(numberText), "%.2f", number);
  engine_drawText(numberText, pos, size, colour);
}

void engine_drawArrow(Vector2 start, Vector2 end, float size, Color colour) {
  if (size <= 0) {
    LOG_ERROR(engine__log, "Invalid size: %d", size);
    return;
  }

  DrawLineV(start, end, colour);

  // Calculate angle of the line
  float angle = atan2f(end.y - start.y, end.x - start.x);

  // Arrowhead points
  Vector2 right = { end.x - size * cosf(angle - PI / 6), end.y - size * sinf(angle - PI / 6) };
  Vector2 left  = { end.x - size * cosf(angle + PI / 6), end.y - size * sinf(angle + PI / 6) };

  DrawLineV(end, right, colour);
  DrawLineV(end, left, colour);
}
