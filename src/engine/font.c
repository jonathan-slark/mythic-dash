#include "engine.h"
#include "engine_internal.h"

#include "log/log.h"

#include <errno.h>   // errno
#include <stdlib.h>  // malloc
#include <string.h>  // strerror

#include <raylib.h>

engine_Font* engine_fontLoad(const char* filepath, int glyphWidth, int glyphHeight) {
  if (filepath == nullptr) {
    LOG_ERROR(engine__log, "Failed to load font: filepath is nullptr");
    return nullptr;
  }
  if (glyphWidth <= 0 || glyphHeight <= 0) {
    LOG_ERROR(engine__log, "Failed to load font: glyphWidth or glyphHeight is <= 0");
    return nullptr;
  }

  engine_Font* font = (engine_Font*) malloc(sizeof(engine_Font));
  if (font == nullptr) {
    char* error = strerror(errno);
    LOG_ERROR(engine__log, "Failed to allocate memory for font: %s", error);
    return nullptr;
  }

  font->texture = LoadTexture(filepath);
  if (font->texture.id == 0) {
    LOG_ERROR(engine__log, "Failed to load font: %s", filepath);
    free(font);
    return nullptr;
  }

  font->glyphWidth  = glyphWidth;
  font->glyphHeight = glyphHeight;
  font->columns     = font->texture.width / glyphWidth;
  font->rows        = font->texture.height / glyphHeight;

  return font;
}

void engine_fontUnload(engine_Font** font) {
  if (font == nullptr || *font == nullptr) {
    LOG_ERROR(engine__log, "Failed to unload font: font is nullptr");
    return;
  }

  UnloadTexture((*font)->texture);
  free(*font);
  *font = nullptr;
}

void engine_fontPrintf([[maybe_unused]] engine_Font* font,
                       [[maybe_unused]] int x,
                       [[maybe_unused]] int y,
                       [[maybe_unused]] const char* format,
                       ...) {}
