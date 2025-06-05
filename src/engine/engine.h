// clang-format Language: C
#pragma once

#include <raylib.h>  // Vector2, KeyboardKey, Color

/**
 * @file engine.h
 * @brief Simple game engine wrapper around Raylib
 */

/**
 * @defgroup Types Engine data types
 * @{
 */

typedef struct engine_Anim    engine_Anim;    /**< Opaque animation type */
typedef struct engine_Sprite  engine_Sprite;  /**< Opaque sprite type */
typedef struct engine_Texture engine_Texture; /**< Opaque texture type */
typedef struct engine_Font    engine_Font;    /**< Opaque font type */

/** @} */

/**
 * @defgroup Core Core engine functions
 * @{
 */

/**
 * @brief Initialize the engine
 *
 * Create a borderless window using the current display size and refresh rate.
 * Rendering is scaled from the native canvas size to the display size.
 *
 * @param nativeWidth Native canvas width
 * @param nativeHeight Native canvas height
 * @param title Window title
 * @param fps Target FPS, 0 for current display refresh rate
 * @return true if successful
 */
[[nodiscard]] bool engine_init(int nativeWidth, int nativeHeight, const char* title, int fps);

/**
 * @brief Shutdown the engine and free resources
 */
void engine_shutdown(void);

/**
 * @brief Get the frame time
 * @return Frame time in seconds
 */
static inline float engine_getFrameTime(void) { return GetFrameTime(); }

/**
 * @brief Check if the window should close
 * @return true if the window should close
 */
static inline bool engine_shouldClose(void) { return WindowShouldClose(); }

/** @} */

/**
 * @defgroup Input Input handling functions
 * @{
 */

/**
 * @brief Check if a key is being held down
 * @param key KeyboardKey to check
 * @return true if key is down
 */
static inline bool engine_isKeyDown(KeyboardKey key) { return IsKeyDown(key); }

/**
 * @brief Check if a key was pressed once
 * @param key KeyboardKey to check
 * @return true if key was pressed
 */
static inline bool engine_isKeyPressed(KeyboardKey key) { return IsKeyPressed(key); }

/**
 * @brief Check if a key was released
 * @param key KeyboardKey to check
 * @return true if key was released
 */
static inline bool engine_isKeyReleased(KeyboardKey key) { return IsKeyReleased(key); }

/** @} */

/**
 * @defgroup Sprites Sprite handling functions
 * @{
 */

/**
 * @brief Create a sprite
 * @param position Sprite position coordinates
 * @param size Sprite width and height
 * @param offset Texture offset
 * @return Pointer to the newly created sprite
 */
[[nodiscard]] engine_Sprite* engine_createSprite(Vector2 position, Vector2 size, Vector2 offset);

/**
 * @brief Create a sprite from a sprite sheet
 * @param position Sprite position coordinates
 * @param size Sprite width and height
 * @param row Row of sprite on the sheet
 * @param col Column of sprite on the sheet
 * @return Pointer to the newly created sprite
 */
[[nodiscard]] engine_Sprite* engine_createSpriteFromSheet(Vector2 position, Vector2 size, int row, int col);

/**
 * @brief Destroy a sprite
 * @param sprite Pointer to sprite pointer (will be set to nullptr)
 */
void engine_destroySprite(engine_Sprite** sprite);

/**
 * @brief Draw a sprite with the given texture
 * @param texture Texture to draw
 * @param sprite Sprite properties
 */
void engine_drawSprite(const engine_Texture* texture, const engine_Sprite* sprite);

/**
 * @brief Set the position of an existing sprite
 * @param sprite Pointer to sprite pointer
 * @param position New sprite position
 */
void engine_spriteSetPos(engine_Sprite* sprite, Vector2 position);

/** @} */

/**
 * @defgroup Anim Animimation handling functions
 * @{
 */

/**
 * @brief Create an anim
 * @param sprite Pointer to sprite object
 * @param row Row in sprite sheet
 * @param startCol First frame to render
 * @param frameCount Number of frames
 * @param frameTime Length of time in seconds to display each frame
 * @return Pointer to the newly created anim
 */
[[nodiscard]] engine_Anim*
engine_createAnim(engine_Sprite* sprite, int row, int startCol, int frameCount, float frameTime);

/**
 * @brief Destroy an anim
 * @param anim Pointer to anim pointer (will be set to nullptr)
 */
void engine_destroyAnim(engine_Anim** anim);

/**
 * @brief Update an animation, called once per game frame
 * @param anim Anim to update
 * @param deltaTime Amount of time elapsed since last game frame
 */
void engine_updateAnim(engine_Anim* anim, float deltaTime);

/** @} */

/**
 * @defgroup Textures Texture handling functions
 * @{
 */

/**
 * @brief Load a texture from file
 * @param filepath Path to the image file
 * @return Pointer to the loaded texture or nullptr on failure
 */
[[nodiscard]] engine_Texture* engine_textureLoad(const char* filepath);

/**
 * @brief Unload a texture and free memory
 * @param texture Pointer to texture pointer (will be set to nullptr)
 */
void engine_textureUnload(engine_Texture** texture);

/** @} */

/**
 * @defgroup Fonts Font handling functions
 * @{
 */

/**
 * @brief Load a bitmap font
 * @param filepath Path to the font image
 * @param glyphWidth Width of each character
 * @param glyphHeight Height of each character
 * @param asciiStart First ASCII character in the font
 * @param asciiEnd Last ASCII character in the font
 * @param glyphSpacing Spacing between characters
 * @return Pointer to the loaded font or nullptr on failure
 */
[[nodiscard]] engine_Font*
engine_fontLoad(const char* filepath, int glyphWidth, int glyphHeight, int asciiStart, int asciiEnd, int glyphSpacing);

/**
 * @brief Unload a font and free memory
 * @param font Pointer to font pointer (will be set to nullptr)
 */
void engine_fontUnload(engine_Font** font);

/**
 * @brief Format and draw text with a font
 * @param font Font to use
 * @param x X position
 * @param y Y position
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void engine_fontPrintf(engine_Font* font, int x, int y, const char* format, ...);

/**
 * @brief Format and draw text with a font (va_list version)
 * @param font Font to use
 * @param x X position
 * @param y Y position
 * @param format Printf-style format string
 * @param args Variable argument list
 */
void engine_fontPrintfV(engine_Font* font, int x, int y, const char* format, va_list args);

/** @} */

/**
 * @defgroup Rendering Rendering functions
 * @{
 */

/**
 * @brief Draw a texture as full background
 * @param background Texture to draw as background
 */
void engine_drawBackground(engine_Texture* background);

/**
 * @brief Draw a rectangle outline
 * @param rect Rectangle to draw
 * @param color Color to use
 */
void engine_drawRectangleOutline(Rectangle rect, Color color);

/**
 * @brief Begin drawing a new frame
 */
static inline void engine_beginFrame(void) { BeginDrawing(); }

/**
 * @brief End drawing the current frame
 */
static inline void engine_endFrame(void) { EndDrawing(); }

/**
 * @brief Clear the screen with a color
 * @param color Color to use
 */
static inline void engine_clearScreen(Color color) { ClearBackground(color); }

/** @} */
