#include "engine.h"
#include "engine_internal.h"

#include <stdlib.h>  // malloc

engine_Texture* engine_textureLoad(const char* filepath) {
  engine_Texture* texture = malloc(sizeof(engine_Texture));
  texture->handle = LoadTexture(filepath);
  return texture;
}

void engine_textureUnload(engine_Texture* texture) {
  UnloadTexture(texture->handle);
  free(texture);
}
