#include "engine.h"
#include "engine_internal.h"

#include <raylib.h>

// --- Renderer functions ---

void engine_beginFrame(void) {
  BeginDrawing();
}

void engine_endFrame(void) {
  EndDrawing();
}

void engine_clearScreen(Color color) {
  ClearBackground(color);
}

void engine_drawSprite(const engine_Texture* texture,
                       const engine_Sprite* sprite) {
  if (texture == nullptr) {
    LOG_WARN(engine_log, "Texture is nullptr");
    return;
  }
  if (sprite == nullptr) {
    LOG_WARN(engine_log, "Sprite is nullptr");
    return;
  }

  int scale = engine__screenState.scale;
  Rectangle src = (Rectangle){sprite->offset.x, sprite->offset.y,
                              sprite->size.x, sprite->size.y};
  Rectangle dst =
      (Rectangle){sprite->position.x * scale, sprite->position.y * scale,
                  sprite->size.x * scale, sprite->size.y * scale};
  DrawTexturePro(texture->handle, src, dst, (Vector2){0, 0}, 0, WHITE);
}

void engine_drawBackground(engine_Texture* background) {
  if (background == nullptr) {
    LOG_WARN(engine_log, "Background is nullptr");
    return;
  }

  DrawTextureEx(background->handle, (Vector2){0, 0}, 0,
                engine__screenState.scale, WHITE);
}
