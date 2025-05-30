#include <raylib.h>
#include "engine.h"
#include "internal.h"

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
