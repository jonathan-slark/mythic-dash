#include "engine.h"
#include "engine_internal.h"

#include <raylib.h>

// --- Renderer functions ---

void engine_drawSprite(const engine_Texture* texture, const engine_Sprite* sprite) {
  if (texture == nullptr) {
    LOG_WARN(engine__log, "Texture is nullptr");
    return;
  }
  if (sprite == nullptr) {
    LOG_WARN(engine__log, "Sprite is nullptr");
    return;
  }

  int scale     = engine__screenState.scale;
  Rectangle src = (Rectangle){sprite->offset.x, sprite->offset.y, sprite->size.x, sprite->size.y};
  Rectangle dst = (Rectangle){sprite->position.x * scale, sprite->position.y * scale, sprite->size.x * scale,
                              sprite->size.y * scale};
  DrawTexturePro(texture->texture, src, dst, (Vector2){0, 0}, 0, WHITE);
}

void engine_drawBackground(engine_Texture* background) {
  if (background == nullptr) {
    LOG_WARN(engine__log, "Background is nullptr");
    return;
  }

  DrawTextureEx(background->texture, (Vector2){0, 0}, 0, engine__screenState.scale, WHITE);
}
