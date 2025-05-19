#include "game.h"

#include "../engine/engine.h"
#include "game_internal.h"

#include <assert.h>
#include <raylib.h>

// --- Constants ---

static const char FILE_BACKGROUND[] = "../../asset/gfx/background.png";
static const char FILE_SPRITES[] = "../../asset/gfx/sprites.png";

// --- Global state ---

static engine_Texture *g_background;
static engine_Texture *g_sprites;
static engine_Sprite g_playerSprite = {.size = {16, 16}, .offset = {0, 0}};

// --- Game functions ---

void game_load(void) {
  g_background = engine_textureLoad(FILE_BACKGROUND);
  g_sprites = engine_textureLoad(FILE_SPRITES);

  game__playerInit();
}

void game_update(void) { game__playerUpdate(); }

void game_draw(void) {
  engine_drawBackground(g_background);
  g_playerSprite.position = game__playerGetPos();
  engine_drawSprite(g_sprites, &g_playerSprite);
}

void game_unload(void) {
  engine_textureUnload(g_background);
  engine_textureUnload(g_sprites);
}
