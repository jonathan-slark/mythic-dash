#include <errno.h>  // errno
#include <raylib.h>
#include <stdio.h>   // vsnprintf
#include <stdlib.h>  // malloc
#include <string.h>  // strerror
#include "engine.h"
#include "internal.h"
#include "log/log.h"

// --- Font functions ---

engine_Font* engine_fontLoad(const char* filepath,
                             int         glyphWidth,
                             int         glyphHeight,
                             int         asciiStart,
                             int         asciiEnd,
                             int         glyphSpacing) {
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

  font->glyphWidth   = glyphWidth;
  font->glyphHeight  = glyphHeight;
  font->columns      = font->texture.width / glyphWidth;
  font->rows         = font->texture.height / glyphHeight;
  font->asciiStart   = asciiStart;
  font->asciiEnd     = asciiEnd;
  font->glyphSpacing = glyphSpacing;

  LOG_INFO(engine__log, "Loaded font: %s, %dx%d, %d columns, %d row%s", filepath, font->glyphWidth, font->glyphHeight,
           font->columns, font->rows, font->rows == 1 ? "" : "s");

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

void engine_fontPrintf(engine_Font* font, int x, int y, const char* format, ...) {
  if (font == nullptr) {
    LOG_ERROR(engine__log, "Failed to print font: font is nullptr");
    return;
  }

  va_list args;
  va_start(args, format);
  engine_fontPrintfV(font, x, y, format, args);
  va_end(args);
}

void engine_fontPrintfV(engine_Font* font, int x, int y, const char* format, va_list args) {
  if (font == nullptr) {
    LOG_ERROR(engine__log, "Failed to print font: font is nullptr");
    return;
  }
  if (font->texture.id == 0) {
    LOG_ERROR(engine__log, "Failed to print font: font texture is not loaded");
    return;
  }
  if (x < 0 || y < 0) {
    LOG_ERROR(engine__log, "Failed to print font: x or y is < 0");
    return;
  }
  if (format == nullptr) {
    LOG_ERROR(engine__log, "Failed to print font: format is nullptr");
    return;
  }

  // Get the size of the formatted string, copy the args so we don't touch the original
  va_list ap;
  va_copy(ap, args);
  int size = vsnprintf(NULL, 0, format, ap);
  va_end(ap);

  char* text = (char*) malloc(size + 1);
  if (text == nullptr) {
    LOG_ERROR(engine__log, "Failed to allocate memory for formatted string");
    return;
  }
  vsnprintf(text, size + 1, format, args);

  for (int i = 0; i <= size; i++) {
    char c = text[i];
    // Only render visible ASCII characters
    if (c < font->asciiStart || c > font->asciiEnd) {
      continue;
    }
    int       charIndex = c - font->asciiStart;
    int       col       = charIndex % font->columns;
    int       row       = charIndex / font->columns;
    int       scale     = engine__screenState.scale;

    Rectangle src       = {.x      = (float) (col * font->glyphWidth + (col + 1) * font->glyphSpacing),
                           .y      = (float) (row * font->glyphHeight + (row + 1) * font->glyphSpacing),
                           .width  = (float) (font->glyphWidth),
                           .height = (float) (font->glyphHeight)};
    Rectangle dst       = {.x      = (float) ((x + i * font->glyphWidth) * scale),
                           .y      = (float) (y * scale),
                           .width  = (float) (font->glyphWidth * scale),
                           .height = (float) (font->glyphHeight * scale)};

    DrawTexturePro(font->texture, src, dst, (Vector2) {0, 0}, 0, WHITE);
  }

  free(text);
}
