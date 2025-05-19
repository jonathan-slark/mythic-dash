#include "game.h"

#include <assert.h>
#include <raylib.h>

#include "../engine/engine.h"
#include "../log/log.h"
#include "game_internal.h"

// --- Constants ---

static const char FILE_BACKGROUND[] = "../../asset/gfx/background.png";
static const char FILE_SPRITES[] = "../../asset/gfx/sprites.png";

// --- Global state ---

static engine_Texture* g_background = nullptr;
static engine_Texture* g_sprites = nullptr;
static engine_Sprite g_playerSprite = {.size = {16, 16}, .offset = {0, 0}};

// --- Game functions ---

bool game_load(void) {
  GAME_TRY(g_background = engine_textureLoad(FILE_BACKGROUND));
  GAME_TRY(g_sprites = engine_textureLoad(FILE_SPRITES));

  game__playerInit();

  return true;
}

void game_update(void) {
  game__playerUpdate();
}

void game_draw(void) {
  engine_drawBackground(g_background);
  g_playerSprite.position = game__playerGetPos();
  engine_drawSprite(g_sprites, &g_playerSprite);
}

void game_unload(void) {
  engine_textureUnload(&g_background);
  engine_textureUnload(&g_sprites);
}
