#include "engine.h"
#include "engine_internal.h"

#include <errno.h>   // errno
#include <stdlib.h>  // malloc
#include <string.h>  // strerror

engine_Texture* engine_textureLoad(const char* filepath) {
  if (filepath == nullptr) {
    LOG_ERROR(engine__log, "Invalid filepath: nullptr");
    return nullptr;
  }

  engine_Texture* texture = (engine_Texture*) malloc(sizeof(engine_Texture));
  if (texture == nullptr) {
    char* error = strerror(errno);
    LOG_ERROR(engine__log, "Failed to allocate memory for texture: %s", error);
    return nullptr;
  }

  texture->texture = LoadTexture(filepath);
  if (texture->texture.id == 0) {
    LOG_ERROR(engine__log, "Failed to load texture: %s", filepath);
    free(texture);
    return nullptr;
  }

  return texture;
}

void engine_textureUnload(engine_Texture** texture) {
  if (texture == nullptr || *texture == nullptr) {
    LOG_WARN(engine__log, "Texture is nullptr");
    return;
  }

  UnloadTexture((*texture)->texture);
  free(*texture);
  *texture = nullptr;
}
