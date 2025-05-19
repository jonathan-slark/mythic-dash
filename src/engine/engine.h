#pragma once

#include <raylib.h> // KeyboardKey, Texture2D, Color

typedef struct EngineScreenState {
  int width;       /**< Width of the screen in pixels */
  int height;      /**< Height of the screen in pixels */
  int refreshRate; /**< Display refresh rate in Hz */
  int scale;       /**< Rendering scale factor */
} EngineScreenState;

typedef struct {
  Texture2D handle;
  int width, height;
} EngineTexture;

typedef struct {
  Texture2D texture;
  int glyphWidth;
  int glyphHeight;
  int columns;
  int rows;
} EngineFont;

extern EngineScreenState gEngineScreenState;

void engine_init(int orgScreenWidth, int orgScreenHeight, const char *title);
bool engine_shouldClose(void);
void engine_beginFrame(void);
void engine_endFrame(void);
void engine_shutdown(void);

int engine_windowGetWidth(void);
int engine_windowGetHeight(void);

bool engine_inputKeyDown(KeyboardKey key);
bool engine_inputKeyPressed(KeyboardKey key);
bool engine_inputKeyReleased(KeyboardKey key);

EngineTexture engine_textureLoad(const char *filepath);
void engine_textureUnload(EngineTexture *texture);
void engine_textureDraw(EngineTexture *texture, int x, int y);

EngineFont engine_fontLoad(const char *path, int glyphWidth, int glyphHeight);
void engine_fontUnload(EngineFont *font);
void engine_fontDrawText(EngineFont *font, const char *text, int x, int y);

void engine_rendererClear(Color color);
void engine_rendererDrawRect(int x, int y, int w, int h, Color color);
