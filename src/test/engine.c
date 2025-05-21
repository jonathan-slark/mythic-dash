// Disable some Windows-specific features to avoid conflicts with raylib
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMINMAX
#define NOUSER
#define NOSERVICE
#define NOCOMM
#define NOKERNEL

#include "../engine/engine.h"
#include <minunit.h>
#include <raylib.h>  // Include raylib directly to avoid redefinition issues
#include <stdarg.h>  // For va_list
#include <stdlib.h>
#include "../engine/engine_internal.h"

// Mock variables for testing
static bool mockWindowShouldClose  = false;
static bool mockInitWindowCalled   = false;
static int mockScreenWidth         = 1280;
static int mockScreenHeight        = 720;
static int mockRefreshRate         = 60;
static int mockCurrentMonitor      = 0;
static bool mockTextureLoadSuccess = true;
static bool mockFontLoadSuccess    = true;
static char* mockLastDrawnText     = NULL;
static float mockFrameTime         = 0.016667f;  // Default value (60 FPS)

// Test resources
static engine_Texture* testTexture = NULL;
static engine_Font* testFont       = NULL;

// --- Mock implementations for Raylib functions ---

bool WindowShouldClose(void) { return mockWindowShouldClose; }

void InitWindow(int width [[maybe_unused]], int height [[maybe_unused]], const char* title [[maybe_unused]]) {
  mockInitWindowCalled = true;
}

int GetScreenWidth(void) { return mockScreenWidth; }

int GetScreenHeight(void) { return mockScreenHeight; }

int GetMonitorRefreshRate(int monitor [[maybe_unused]]) { return mockRefreshRate; }

int GetCurrentMonitor(void) { return mockCurrentMonitor; }

void SetTargetFPS(int fps [[maybe_unused]]) {
  // Mock implementation
}

void ToggleBorderlessWindowed(void) {
  // Mock implementation
}

void HideCursor(void) {
  // Mock implementation
}

void CloseWindow(void) {
  // Mock implementation
}

void SetTraceLogLevel(int logLevel [[maybe_unused]]) {
  // Mock implementation
}

void SetTraceLogCallback([[maybe_unused]] void (*callback)(int, const char*, va_list)) {
  // Mock implementation
}

Texture2D LoadTexture(const char* fileName [[maybe_unused]]) {
  Texture2D texture = {};
  if (mockTextureLoadSuccess) {
    texture.id     = 1;  // Non-zero ID means success
    texture.width  = 64;
    texture.height = 64;
  }
  return texture;
}

void UnloadTexture(Texture2D texture [[maybe_unused]]) {
  // Mock implementation
}

void DrawTexturePro(Texture2D texture [[maybe_unused]],
                    Rectangle sourceRec [[maybe_unused]],
                    Rectangle destRec [[maybe_unused]],
                    Vector2 origin [[maybe_unused]],
                    float rotation [[maybe_unused]],
                    Color tint [[maybe_unused]]) {
  // Mock implementation
}

void DrawTextureEx(Texture2D texture [[maybe_unused]],
                   Vector2 position [[maybe_unused]],
                   float rotation [[maybe_unused]],
                   float scale [[maybe_unused]],
                   Color tint [[maybe_unused]]) {
  // Mock implementation
}

void BeginDrawing(void) {
  // Mock implementation
}

void EndDrawing(void) {
  // Mock implementation
}

void ClearBackground(Color color [[maybe_unused]]) {
  // Mock implementation
}

float GetFrameTime(void) { return mockFrameTime; }

// Setup and teardown
static void test_setup(void) {
  // Reset mock state
  mockWindowShouldClose  = false;
  mockInitWindowCalled   = false;
  mockScreenWidth        = 1280;
  mockScreenHeight       = 720;
  mockRefreshRate        = 60;
  mockCurrentMonitor     = 0;
  mockTextureLoadSuccess = true;
  mockFontLoadSuccess    = true;
  mockFrameTime          = 0.016667f;  // 60 FPS

  // Initialize engine with test values
  engine_init(320, 180, "Test");
}

static void test_teardown(void) {
  // Clean up
  engine_shutdown();

  if (testTexture != NULL) {
    engine_textureUnload(&testTexture);
  }

  if (testFont != NULL) {
    engine_fontUnload(&testFont);
  }

  free(mockLastDrawnText);
  mockLastDrawnText = NULL;
}

// Test initialization and shutdown
MU_TEST(test_engine_init_success) {
  // Reset from setup
  engine_shutdown();
  mockInitWindowCalled = false;

  // Test
  bool result          = engine_init(320, 180, "Test");

  mu_assert(result, "Engine initialization should succeed");
  mu_assert(mockInitWindowCalled, "InitWindow should be called");
}

MU_TEST(test_engine_init_invalid_size) {
  // Reset from setup
  engine_shutdown();

  // Test
  bool result = engine_init(0, 180, "Test");

  mu_assert(!result, "Engine initialization should fail with width <= 0");

  // Test height
  result = engine_init(320, 0, "Test");

  mu_assert(!result, "Engine initialization should fail with height <= 0");
}

MU_TEST(test_engine_init_null_title) {
  // Reset from setup
  engine_shutdown();

  // Test
  bool result = engine_init(320, 180, NULL);

  mu_assert(!result, "Engine initialization should fail with NULL title");
}

// Test window management
MU_TEST(test_engine_shouldClose) {
  mockWindowShouldClose = false;
  mu_assert(!engine_shouldClose(), "Should return false when window is not set to close");

  mockWindowShouldClose = true;
  mu_assert(engine_shouldClose(), "Should return true when window is set to close");
}

// Test texture loading
MU_TEST(test_texture_load_success) {
  mockTextureLoadSuccess = true;
  testTexture            = engine_textureLoad("test.png");

  mu_assert(testTexture != NULL, "Texture loading should succeed");
  mu_assert(testTexture->texture.id != 0, "Loaded texture should have non-zero ID");
}

MU_TEST(test_texture_load_null_path) {
  testTexture = engine_textureLoad(NULL);

  mu_assert(testTexture == NULL, "Texture loading should fail with NULL path");
}

MU_TEST(test_texture_load_failure) {
  mockTextureLoadSuccess = false;
  testTexture            = engine_textureLoad("nonexistent.png");

  mu_assert(testTexture == NULL, "Texture loading should fail with invalid path");
}

MU_TEST(test_texture_unload) {
  mockTextureLoadSuccess = true;
  testTexture            = engine_textureLoad("test.png");

  engine_textureUnload(&testTexture);
  mu_assert(testTexture == NULL, "Texture should be NULL after unloading");

  // Test with NULL pointer
  engine_textureUnload(NULL);
  engine_textureUnload(&testTexture);  // testTexture is already NULL
}

// Test font operations
MU_TEST(test_font_load_success) {
  mockFontLoadSuccess = true;
  testFont            = engine_fontLoad("test_font.png", 8, 8, 32, 127, 1);

  mu_assert(testFont != NULL, "Font loading should succeed");
  mu_assert(testFont->texture.id != 0, "Loaded font texture should have non-zero ID");
  mu_assert_int_eq(8, testFont->glyphWidth);
  mu_assert_int_eq(8, testFont->glyphHeight);
  mu_assert_int_eq(32, testFont->asciiStart);
  mu_assert_int_eq(127, testFont->asciiEnd);
  mu_assert_int_eq(1, testFont->glyphSpacing);
}

MU_TEST(test_font_load_null_path) {
  testFont = engine_fontLoad(NULL, 8, 8, 32, 127, 1);

  mu_assert(testFont == NULL, "Font loading should fail with NULL path");
}

MU_TEST(test_font_load_invalid_dimensions) {
  testFont = engine_fontLoad("test_font.png", 0, 8, 32, 127, 1);

  mu_assert(testFont == NULL, "Font loading should fail with width <= 0");

  testFont = engine_fontLoad("test_font.png", 8, 0, 32, 127, 1);

  mu_assert(testFont == NULL, "Font loading should fail with height <= 0");
}

MU_TEST(test_font_unload) {
  mockFontLoadSuccess = true;
  testFont            = engine_fontLoad("test_font.png", 8, 8, 32, 127, 1);

  engine_fontUnload(&testFont);
  mu_assert(testFont == NULL, "Font should be NULL after unloading");

  // Test with NULL pointer
  engine_fontUnload(NULL);
  engine_fontUnload(&testFont);  // testFont is already NULL
}

// Test drawing functions
MU_TEST(test_draw_sprite) {
  // This test just ensures the function doesn't crash
  engine_Sprite sprite = {.position = {10, 10}, .size = {16, 16}, .offset = {0, 0}};

  // Test with NULL texture
  engine_drawSprite(NULL, &sprite);

  // Test with NULL sprite
  mockTextureLoadSuccess = true;
  testTexture            = engine_textureLoad("test.png");
  engine_drawSprite(testTexture, NULL);

  // Test with valid arguments
  engine_drawSprite(testTexture, &sprite);
}

MU_TEST(test_draw_background) {
  // Test with NULL texture
  engine_drawBackground(NULL);

  // Test with valid texture
  mockTextureLoadSuccess = true;
  testTexture            = engine_textureLoad("test.png");
  engine_drawBackground(testTexture);
}

// Test frame management
MU_TEST(test_frame_management) {
  // These just test that the functions don't crash
  engine_beginFrame();
  engine_clearScreen((Color) {0, 0, 0, 255});
  engine_endFrame();
}

// Test input handling
MU_TEST(test_input_handling) {
  // These are thin wrappers around Raylib functions, we just test they don't crash
  engine_isKeyDown(KEY_SPACE);
  engine_isKeyPressed(KEY_ENTER);
  engine_isKeyReleased(KEY_ESCAPE);
}

// Test time functions
MU_TEST(test_engine_getFrameTime) {
  // Test default frame time (60 FPS)
  mockFrameTime = 0.016667f;
  mu_assert_double_eq(0.016667f, engine_getFrameTime());

  // Test different frame time (30 FPS)
  mockFrameTime = 0.033333f;
  mu_assert_double_eq(0.033333f, engine_getFrameTime());

  // Test zero frame time
  mockFrameTime = 0.0f;
  mu_assert_double_eq(0.0f, engine_getFrameTime());
}

// Test suite
MU_TEST_SUITE(test_engine_suite) {
  MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

  // Initialization tests
  MU_RUN_TEST(test_engine_init_success);
  MU_RUN_TEST(test_engine_init_invalid_size);
  MU_RUN_TEST(test_engine_init_null_title);

  // Window management tests
  MU_RUN_TEST(test_engine_shouldClose);

  // Texture tests
  MU_RUN_TEST(test_texture_load_success);
  MU_RUN_TEST(test_texture_load_null_path);
  MU_RUN_TEST(test_texture_load_failure);
  MU_RUN_TEST(test_texture_unload);

  // Font tests
  MU_RUN_TEST(test_font_load_success);
  MU_RUN_TEST(test_font_load_null_path);
  MU_RUN_TEST(test_font_load_invalid_dimensions);
  MU_RUN_TEST(test_font_unload);

  // Drawing tests
  MU_RUN_TEST(test_draw_sprite);
  MU_RUN_TEST(test_draw_background);
  MU_RUN_TEST(test_frame_management);

  // Input tests
  MU_RUN_TEST(test_input_handling);

  // Time tests
  MU_RUN_TEST(test_engine_getFrameTime);
}

// Main function
int main(void) {
  MU_RUN_SUITE(test_engine_suite);
  MU_REPORT();

  return MU_EXIT_CODE;
}
