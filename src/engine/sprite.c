#include <errno.h>  // errno
#include <raylib.h>
#include <stdlib.h>  // malloc, free
#include <string.h>  // strerror
#include "../log/log.h"
#include "engine.h"
#include "internal.h"

// --- Sprite functions ---

engine_Sprite* engine_createSprite(Vector2 position, Vector2 size, Vector2 offset) {
  if (size.x <= 0 || size.y <= 0) {
    LOG_ERROR(engine__log, "Failed to create sprite: invalid size");
    return nullptr;
  }
  if (offset.x < 0 || offset.y < 0) {
    LOG_ERROR(engine__log, "Failed to create sprite: invalid offset");
    return nullptr;
  }

  engine_Sprite* sprite = (engine_Sprite*) malloc(sizeof(engine_Sprite));
  if (sprite == nullptr) {
    char* error = strerror(errno);
    LOG_ERROR(engine__log, "Failed to allocate memory for texture: %s", error);
    return nullptr;
  }

  sprite->position = position;
  sprite->size     = size;
  sprite->offset   = offset;

  return sprite;
}

engine_Sprite* engine_createSpriteFromSheet(Vector2 position, Vector2 size, int row, int col) {
  if (size.x <= 0 || size.y <= 0) {
    LOG_ERROR(engine__log, "Failed to create sprite: invalid size");
    return nullptr;
  }
  if (row < 0 || col < 0) {
    LOG_ERROR(engine__log, "Failed to create sprite: row or col is < 0");
    return nullptr;
  }

  Vector2 offset = engine__getSpriteSheetOffset(row, col, size);
  return engine_createSprite(position, size, offset);
}

void engine_destroySprite(engine_Sprite** sprite) {
  if (sprite == nullptr || *sprite == nullptr) {
    LOG_WARN(engine__log, "Sprite is nullptr");
    return;
  }

  free(*sprite);
  *sprite = nullptr;
}

void engine_drawSprite(const engine_Texture* texture, const engine_Sprite* sprite) {
  if (texture == nullptr) {
    LOG_WARN(engine__log, "Texture is nullptr");
    return;
  }
  if (sprite == nullptr) {
    LOG_WARN(engine__log, "Sprite is nullptr");
    return;
  }

  int       scale = engine__screenState.scale;
  Rectangle src   = (Rectangle) {
      .x = sprite->offset.x, .y = sprite->offset.y, .width = sprite->size.x, .height = sprite->size.y
  };
  Rectangle dst = (Rectangle) { .x      = sprite->position.x * scale,
                                .y      = sprite->position.y * scale,
                                .width  = sprite->size.x * scale,
                                .height = sprite->size.y * scale };
  DrawTexturePro(texture->texture, src, dst, (Vector2) { 0, 0 }, 0, WHITE);
}

void engine_spriteSetPos(engine_Sprite* sprite, Vector2 position) {
  if (sprite == nullptr) {
    LOG_WARN(engine__log, "Sprite is nullptr");
    return;
  }

  sprite->position = position;
}
