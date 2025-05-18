#include "game.h"

#include "common.h"
#include "player.h"

#include <assert.h>
#include <raylib.h>

// --- Types ---

typedef struct Sprite {
  Vector2 size;
  Vector2 offset;
} Sprite;

// --- Constants ---

static const char   FILE_BACKGROUND[] = "../../asset/gfx/background.png";
static const char   FILE_SPRITES[]    = "../../asset/gfx/sprites.png";
static const Sprite gPlayerSprite     = {
      .size   = { 16, 16 },
      .offset = {  0,  0 }
};
const Vector2 MAZE_ORIGIN = { 132, 15 };

// --- Global state ---

static Texture gBackground;
static Texture gSprites;

// --- Helper functions ---

static void drawBackground(Texture* background) {
  assert(background != nullptr);
  DrawTextureEx(*background, (Vector2) { 0, 0 }, 0, gScreenState.scale, WHITE);
}

static void drawSprite(const Sprite* sprite, Vector2 pos) {
  assert(sprite != nullptr);
  int       scale = gScreenState.scale;
  Rectangle src   = (Rectangle) { sprite->offset.x, sprite->offset.y, sprite->size.x, sprite->size.y };
  Rectangle dst   = (Rectangle) {
    (pos.x + MAZE_ORIGIN.x) * scale,
    (pos.y + MAZE_ORIGIN.y) * scale,
    sprite->size.x * scale,
    sprite->size.y * scale
  };
  DrawTexturePro(gSprites, src, dst, (Vector2) { 0, 0 }, 0, WHITE);
}

// --- Game functions ---

void game_load(void) {
  gBackground = LoadTexture(FILE_BACKGROUND);
  gSprites    = LoadTexture(FILE_SPRITES);

  player_init();
}

void game_update(void) {
  player_update();
}

void game_draw(void) {
  drawBackground(&gBackground);
  drawSprite(&gPlayerSprite, player_getPos());
}

void game_unload(void) {
  UnloadTexture(gBackground);
  UnloadTexture(gSprites);
}
