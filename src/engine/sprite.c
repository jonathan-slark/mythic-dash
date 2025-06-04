#include <errno.h>  // errno
#include <raylib.h>
#include <stdlib.h>  // malloc, free
#include <string.h>  // strerror
#include "../log/log.h"
#include "engine.h"
#include "internal.h"

// --- Types ---

typedef struct engine_Sprite {
  Vector2 position; /**< Position coordinates */
  Vector2 size;     /**< Width and height */
  Vector2 offset;   /**< Texture offset */
} engine_Sprite;

// --- Helper functions ---

static inline Vector2 getSpriteSheetOffset(int row, int col, Vector2 spriteSize) {
  return (Vector2) { .x = col * spriteSize.x, .y = row * spriteSize.y };
}

// --- Sprite functions ---

engine_Sprite* engine_createSprite(Vector2 position, Vector2 size, Vector2 offset) {
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
  Vector2 offset = getSpriteSheetOffset(row, col, size);
  return engine_createSprite(position, size, offset);
}

void engine_destroySprite(engine_Sprite** sprite) {
  if (sprite == nullptr || *sprite == nullptr) {
    LOG_WARN(engine__log, "Sprite is nullptr");
    return;
  }

  free(*sprite);
  sprite = nullptr;
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
  Rectangle src   = (Rectangle) { sprite->offset.x, sprite->offset.y, sprite->size.x, sprite->size.y };
  Rectangle dst   = (Rectangle) { sprite->position.x * scale, sprite->position.y * scale, sprite->size.x * scale,
                                  sprite->size.y * scale };
  DrawTexturePro(texture->texture, src, dst, (Vector2) { 0, 0 }, 0, WHITE);
}
