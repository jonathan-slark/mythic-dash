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

// --- Constants ---

extern const int LOG_LEVEL_RAYLIB;

// --- Engine functions (engine.c) ---

bool engine_init(int orgScreenWidth, int orgScreenHeight, const char* title);
bool engine_shouldClose(void);
void engine_shutdown(void);

// --- Input functions (input.c) ---

bool engine_isKeyDown(KeyboardKey key);
bool engine_isKeyPressed(KeyboardKey key);
bool engine_isKeyReleased(KeyboardKey key);

// --- Texture functions (texture.c) ---

engine_Texture* engine_textureLoad(const char* filepath);
void engine_textureUnload(engine_Texture** texture);

// --- Font functions (font.c) ---

engine_Font* engine_fontLoad(const char* path, int glyphWidth, int glyphHeight);
void engine_fontUnload(engine_Font** font);
void engine_fontDrawText(engine_Font* font, const char* text, int x, int y);

// --- Renderer functions (renderer.c) ---

void engine_beginFrame(void);
void engine_endFrame(void);
void engine_clearScreen(Color color);
void engine_drawSprite(const engine_Texture* texture, const engine_Sprite* sprite);
void engine_drawBackground(engine_Texture* background);
