#include "draw.h"
#include <assert.h>
#include <raylib.h>
#include <stdarg.h>
#include "../asset/asset.h"
#include "../creature/creature.h"
#include "../internal.h"
#include "../player/player.h"
#include "engine/engine.h"

// --- Constants ---

const char*          PLAYER_STATE_STRINGS[PLAYER_STATE_COUNT] = { "NORMAL", "SWORD", "DEAD" };
static const Color   CREATURE_DEAD_COLOUR                     = { 255, 255, 255, 100 };
static const Vector2 PLAYER_COOLDOWN_OFFSET                   = { 5, -8 };
static const Vector2 CREATURE_SCORE_OFFSET                    = { 1, -8 };

constexpr Color        TEXT_COLOUR       = { 255, 255, 255, 255 };
constexpr Color        SHADOW_COLOUR     = { 32, 32, 32, 255 };
static draw_Text       SWORD_TIMER       = { "%d", 0, 0, TEXT_COLOUR, FONT_TINY };
static draw_Text       CREATURE_SCORE    = { "%d", 0, 0, TEXT_COLOUR, FONT_TINY };
static const draw_Text SCORE_TEXT        = { "Score: %d", 8, 0, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text LEVEL_TEXT        = { "Level: %02d / ?", 394, 0, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text EXTRA_LIFE_TEXT   = { "@ %d", 428, 252, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text TITLE_TEXT        = { "Mythic Dash", 190, 40, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text PLAYER_READY_TEXT = { "Get Ready!", 210, 100, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text GAME_OVER_TEXT    = { "Game over!", 210, 100, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text SPACE_TEXT        = { "Press space", 206, 188, TEXT_COLOUR, FONT_NORMAL };

// --- Draw functions ---

void draw_text(draw_Text text, ...) {
  engine_Font* font = text.fontSize == FONT_TINY ? asset_getFontTiny() : asset_getFont();
  va_list      args;
  va_start(args, text);
  engine_fontPrintfV(font, text.xPos, text.yPos, text.colour, text.format, args);
  va_end(args);
}

void draw_shadowText(draw_Text text, ...) {
  engine_Font* font = text.fontSize == FONT_TINY ? asset_getFontTiny() : asset_getFont();
  va_list      args;
  va_start(args, text);
  engine_fontPrintfV(font, text.xPos + 1, text.yPos + 1, SHADOW_COLOUR, text.format, args);
  engine_fontPrintfV(font, text.xPos, text.yPos, text.colour, text.format, args);
  va_end(args);
}

void draw_resetPlayer(void) {
  game_PlayerState state = player_getState();
  Vector2          pos   = POS_ADJUST(player_getPos());
  game_Dir         dir   = player_getDir();
  engine_resetAnim(asset_getPlayerAnim(state, dir));
  engine_spriteSetPos(asset_getPlayerSprite(state), pos);
}

void draw_resetCreatures(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    Vector2  pos = Vector2Add(POS_ADJUST(creature_getPos(i)), asset_getCreatureOffset(i));
    game_Dir dir = creature_getDir(i);
    engine_resetAnim(asset_getCreatureAnim(i, dir));
    engine_spriteSetPos(asset_getCreateSprite(i), pos);
  }
}

void draw_updatePlayer(float frameTime, float slop) {
  assert(frameTime >= 0.0f);
  assert(slop >= 0.0f);

  static game_PlayerState prevState = PLAYER_NORMAL;
  game_PlayerState        state     = player_getState();
  Vector2                 pos       = POS_ADJUST(player_getPos());
  static game_Dir         prevDir   = DIR_NONE;
  game_Dir                dir       = player_getDir();

  if (state != prevState || dir != prevDir) {
    if (state != prevState)
      LOG_TRACE(
          game_log, "Player state changed from %s to %s", PLAYER_STATE_STRINGS[prevState], PLAYER_STATE_STRINGS[state]
      );
    if (prevDir != DIR_NONE && dir != prevDir)
      LOG_TRACE(game_log, "Player direction changed from %s to %s", DIR_STRINGS[prevDir], DIR_STRINGS[dir]);
    engine_resetAnim(asset_getPlayerAnim(state, dir));
    prevState = state;
    prevDir   = dir;
  }

  engine_spriteSetPos(asset_getPlayerSprite(state), pos);

  if (player_isMoving() || state == PLAYER_SWORD || state == PLAYER_DEAD) {
    engine_updateAnim(asset_getPlayerAnim(state, dir), frameTime);
  }
}

void draw_updateCreatures(float frameTime, float slop) {
  assert(frameTime >= 0.0f);
  assert(slop >= 0.0f);

  for (int i = 0; i < CREATURE_COUNT; i++) {
    Vector2 pos = Vector2Add(POS_ADJUST(creature_getPos(i)), asset_getCreatureOffset(i));
    engine_spriteSetPos(asset_getCreateSprite(i), pos);
    engine_updateAnim(asset_getCreatureAnim(i, creature_getDir(i)), frameTime);
  }
}

void draw_player(void) {
  float swordTimer = player_getSwordTimer();

  bool flash = false;
  if (swordTimer > 0.0f && swordTimer < 1.0f) flash = ((int) (swordTimer * 10) % 2) == 0;
  Color colour = flash ? BLACK : WHITE;
  engine_drawSprite(asset_getPlayerSpriteSheet(), asset_getPlayerSprite(player_getState()), colour);

  for (int i = 0; i < player_getLives() - 1; i++) {
    engine_drawSprite(asset_getPlayerSpriteSheet(), asset_getPlayerLivesSprite(i), WHITE);
  }

  if (swordTimer > 0.0f) {
    Vector2 pos      = Vector2Add(POS_ADJUST(player_getPos()), PLAYER_COOLDOWN_OFFSET);
    SWORD_TIMER.xPos = pos.x;
    SWORD_TIMER.yPos = pos.y;
    draw_text(SWORD_TIMER, (int) ceilf(swordTimer));
  }
}

void draw_creatures(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    Color colour = creature_isFrightened(i) ? BLUE : creature_isDead(i) ? CREATURE_DEAD_COLOUR : WHITE;
    engine_drawSprite(asset_getCreatureSpriteSheet(), asset_getCreateSprite(i), colour);

    int score = creature_getScore(i);
    if (score > 0.0f) {
      Vector2 pos = Vector2Add(POS_ADJUST(creature_getPos(i)), CREATURE_SCORE_OFFSET);
      engine_fontPrintf(asset_getFontTiny(), pos.x, pos.y, WHITE, "%d", score);
      CREATURE_SCORE.xPos = pos.x;
      CREATURE_SCORE.yPos = pos.y;
      draw_text(CREATURE_SCORE, score);
    }
  }
}

void draw_interface(void) {
  draw_text(SCORE_TEXT, player_getScore());
  draw_text(LEVEL_TEXT, game_getLevel());
}

void draw_nextLife(void) {
  engine_drawSprite(asset_getPlayerSpriteSheet(), asset_getPlayerNextLifeSprite(), WHITE);
  draw_text(EXTRA_LIFE_TEXT, player_getNextExtraLifeScore());
}

void draw_title(void) { draw_text(TITLE_TEXT); }

void draw_ready(void) {
  draw_shadowText(PLAYER_READY_TEXT);
  draw_shadowText(SPACE_TEXT);
}

void draw_gameOver(void) {
  draw_shadowText(GAME_OVER_TEXT);
  draw_shadowText(SPACE_TEXT);
}
