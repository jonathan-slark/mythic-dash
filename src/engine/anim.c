#include <errno.h>   // errno
#include <stdlib.h>  // malloc, free
#include <string.h>  // strerror
#include "../log/log.h"
#include "engine.h"
#include "internal.h"
#include "log/log.h"

// --- Types ---

typedef struct engine_Anim {
  engine_Sprite* sprite;       /**< The sprite to update */
  int            startFrame;   /**< Starting frame index (column in sheet) */
  int            frameCount;   /**< How many frames total */
  int            row;          /**< Which row in the sheet */
  float          frameTime;    /**< Time per frame (in seconds) */
  float          timer;        /**< Internal timer */
  int            currentFrame; /**< Which frame we're on */
} engine_Anim;

// --- Anim functions ---

engine_Anim* engine_createAnim(engine_Sprite* sprite, int row, int startFrame, int frameCount, float frameTime) {
  if (sprite == nullptr) {
    LOG_ERROR(engine__log, "Failed to create anim: sprite is nullptr");
    return nullptr;
  }
  if (row < 0 || startFrame < 0 || frameCount < 0) {
    LOG_ERROR(engine__log, "Failed to create sprite: negative parameter");
    return nullptr;
  }
  if (frameTime <= 0.0f) {
    LOG_ERROR(engine__log, "Failed to create sprite: frameTime <= 0");
    return nullptr;
  }

  engine_Anim* anim = (engine_Anim*) malloc(sizeof(engine_Anim));
  if (anim == nullptr) {
    char* error = strerror(errno);
    LOG_ERROR(engine__log, "Failed to allocate memory for anim: %s", error);
    return nullptr;
  }

  anim->sprite       = sprite;
  anim->row          = row;
  anim->startFrame   = startFrame;
  anim->frameCount   = frameCount;
  anim->frameTime    = frameTime;
  anim->timer        = 0.0f;
  anim->currentFrame = 0;

  return anim;
}

void engine_destroyAnim(engine_Sprite** anim) {
  if (anim == nullptr || *anim == nullptr) {
    LOG_WARN(engine__log, "Anim is nullptr");
    return;
  }

  free(*anim);
  anim = nullptr;
}

void engine_updateAnim(engine_Anim* anim, float deltaTime) {
  if (anim == nullptr || anim->sprite == nullptr) {
    LOG_WARN(engine__log, "Anim or sprite are nullptr");
    return;
  }
  if (deltaTime < 0.0f) {
    LOG_WARN(engine__log, "Invalid deltatime");
    return;
  }

  anim->timer += deltaTime;
  if (anim->timer >= anim->frameTime) {
    anim->timer          -= anim->frameTime;
    anim->currentFrame    = (anim->currentFrame + 1) % anim->frameCount;

    int     col           = anim->startFrame + anim->currentFrame;
    Vector2 offset        = engine__getSpriteSheetOffset(anim->row, col, anim->sprite->size);
    anim->sprite->offset  = offset;
  }
}
