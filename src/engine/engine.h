// clang-format Language: C
#pragma once

#include <raylib.h>  // Vector2, KeyboardKey, Color

// --- Types ---

typedef struct engine_Sprite {
  Vector2 position;
  Vector2 size;
  Vector2 offset;
} engine_Sprite;

typedef struct engine_Texture engine_Texture;
typedef struct engine_Font engine_Font;

// --- Engine functions (engine.c) ---

bool engine_init(int nativeWidth, int nativeHeight, const char* title);
void engine_shutdown(void);

static inline bool engine_shouldClose(void) { return WindowShouldClose(); }

// --- Input functions ---

static inline bool engine_isKeyDown(KeyboardKey key) { return IsKeyDown(key); }
static inline bool engine_isKeyPressed(KeyboardKey key) { return IsKeyPressed(key); }
static inline bool engine_isKeyReleased(KeyboardKey key) { return IsKeyReleased(key); }

// --- Texture functions (texture.c) ---

engine_Texture* engine_textureLoad(const char* filepath);
void engine_textureUnload(engine_Texture** texture);

// --- Font functions (font.c) ---

engine_Font* engine_fontLoad(const char* filepath, int glyphWidth, int glyphHeight);
void engine_fontUnload(engine_Font** font);
void engine_fontPrintf(engine_Font* font, int x, int y, const char* format, ...);
void engine_fontPrintfV(engine_Font* font, int x, int y, const char* format, va_list args);

// --- Renderer functions (renderer.c) ---

void engine_drawSprite(const engine_Texture* texture, const engine_Sprite* sprite);
void engine_drawBackground(engine_Texture* background);

static inline void engine_beginFrame(void) { BeginDrawing(); }
static inline void engine_endFrame(void) { EndDrawing(); }
static inline void engine_clearScreen(Color color) { ClearBackground(color); }
