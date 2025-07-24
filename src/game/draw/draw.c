#include "draw.h"
#include <assert.h>
#include <engine/engine.h>
#include <raylib.h>
#include <raymath.h>
#include <stdarg.h>
#include "../asset/asset.h"
#include "../creature/creature.h"
#include "../internal.h"
#include "../player/player.h"
#include "../scores/scores.h"
#include "game/game.h"

// --- Constants ---

const char*          PLAYER_STATE_STRINGS[PLAYER_STATE_COUNT] = { "NORMAL", "SWORD", "DEAD" };
static const Color   CREATURE_DEAD_COLOUR                     = { 255, 255, 255, 100 };
static const Vector2 PLAYER_COOLDOWN_OFFSET                   = { 5, -8 };
static const Vector2 CREATURE_SCORE_OFFSET                    = { 5, -8 };

static const Rectangle LEVEL_CLEAR_BG_RECTANGLE = { 143, 90, 194, 90 };
static const Color     LEVEL_CLEAR_BG_COLOUR    = { 64, 64, 64, 200 };
static const Color     LEVEL_CLEAR_BG_BORDER    = { 255, 255, 255, 200 };

static const Rectangle GAME_WON_BG_RECTANGLE = { 130, 90, 220, 90 };
static const Color     GAME_WON_BG_COLOUR    = { 64, 64, 64, 200 };
static const Color     GAME_WON_BG_BORDER    = { 255, 255, 255, 200 };

constexpr Color        TEXT_COLOUR            = { 255, 255, 255, 255 };
constexpr Color        RECORD_COLOUR          = { 253, 255, 0, 255 };
constexpr Color        SHADOW_COLOUR          = { 32, 32, 32, 255 };
static draw_Text       SWORD_TIMER            = { "%d", 0, 0, TEXT_COLOUR, FONT_TINY };
static draw_Text       CREATURE_SCORE         = { "%d", 0, 0, TEXT_COLOUR, FONT_TINY };
static const draw_Text SCORE_TEXT             = { "Score: %d", 8, 0, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text LEVEL_TEXT             = { "Level: %02d / %02d", 386, 0, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text EXTRA_LIFE_TEXT        = { "@ %d", 428, 252, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text TITLE_TEXT             = { "Mythic Dash", 190, 40, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text LEVEL_CLEAR_TEXT       = { "Level clear!", 153, 100, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text LEVEL_TIME_TEXT        = { "This level's time:  %s", 153, 120, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text LEVEL_SCORE_TEXT       = { "This level's score: %d", 153, 130, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text TIME_RECORD_TEXT       = { "New record time!", 153, 150, RECORD_COLOUR, FONT_NORMAL };
static const draw_Text SCORE_RECORD_TEXT      = { "New record score!", 153, 160, RECORD_COLOUR, FONT_NORMAL };
static const draw_Text GAME_OVER_TEXT         = { "Game over!", 210, 100, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text GAME_WON_TEXT          = { "Game won!", 140, 100, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text TOTAL_RUN_TIME_TEXT    = { "This run's total time:  %s", 140, 120, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text TOTal_RUN_SCORE_TEXT   = { "This run's total score: %d", 140, 130, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text TOTAL_RUN_TIME_RECORD  = { "New record total time!", 140, 150, RECORD_COLOUR, FONT_NORMAL };
static const draw_Text TOTAl_RUN_SCORE_RECORD = { "New record total score!", 140, 160, RECORD_COLOUR, FONT_NORMAL };
static const draw_Text SPACE_TEXT             = { "Press space", 206, 188, TEXT_COLOUR, FONT_NORMAL };
static const draw_Text PLAYER_READY_TEXT[DIFFICULTY_COUNT] = {
  {        "Get ready!", 210, 100, TEXT_COLOUR, FONT_NORMAL },
  {     "Player ready!", 201, 100, TEXT_COLOUR, FONT_NORMAL },
  { "Ready Player One!", 189, 100, TEXT_COLOUR, FONT_NORMAL },
};
static const draw_Text DIFFICULTY_TEXT[DIFFICULTY_COUNT] = {
  {        "Easy", 228, 252, TEXT_COLOUR, FONT_NORMAL },
  {      "Normal", 222, 252, TEXT_COLOUR, FONT_NORMAL },
  { "Arcade Mode", 206, 252, TEXT_COLOUR, FONT_NORMAL }
};

// --- Draw functions ---

void draw_text(draw_Text text, ...) {
  engine_Font* font = text.fontSize == FONT_TINY ? asset_getFontTiny() : asset_getFont();
  va_list      args;
  va_start(args, text);
  engine_fontPrintfV(font, text.xPos, text.yPos, text.colour, text.format, args);
  va_end(args);
}

int draw_getTextOffset(int number) {
  if (number >= 1000) return -6;
  if (number >= 100) return -4;
  if (number >= 10) return -2;
  return 0;
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
    Vector2  pos        = Vector2Add(POS_ADJUST(creature_getPos(i)), asset_getCreatureOffset(i));
    game_Dir dir        = creature_getDir(i);
    int      creatureID = i + game_getLevel() * CREATURE_COUNT;
    engine_resetAnim(asset_getCreatureAnim(creatureID, dir));
    engine_spriteSetPos(asset_getCreateSprite(creatureID), pos);
  }
}

void draw_updatePlayer(double frameTime, float slop) {
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

  if (player_isMoving() || state == PLAYER_SWORD || state == PLAYER_DEAD || state == PLAYER_FALLING) {
    engine_updateAnim(asset_getPlayerAnim(state, dir), frameTime);
  }
}

void draw_updateCreatures(double frameTime, float slop) {
  assert(frameTime >= 0.0f);
  assert(slop >= 0.0f);

  for (int i = 0; i < CREATURE_COUNT; i++) {
    int     creatureID = i + game_getLevel() * CREATURE_COUNT;
    Vector2 pos        = Vector2Add(POS_ADJUST(creature_getPos(i)), asset_getCreatureOffset(creatureID));
    engine_spriteSetPos(asset_getCreateSprite(creatureID), pos);
    engine_updateAnim(asset_getCreatureAnim(creatureID, creature_getDir(i)), frameTime);
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
    int     timer    = (int) ceilf(swordTimer);
    Vector2 pos      = Vector2Add(POS_ADJUST(player_getPos()), PLAYER_COOLDOWN_OFFSET);
    SWORD_TIMER.xPos = pos.x + draw_getTextOffset(timer);
    SWORD_TIMER.yPos = pos.y;
    draw_text(SWORD_TIMER, timer);
  }
}

void draw_creatures(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    Color colour     = creature_isFrightened(i) ? BLUE : creature_isDead(i) ? CREATURE_DEAD_COLOUR : WHITE;
    int   creatureID = i + game_getLevel() * CREATURE_COUNT;
    engine_drawSprite(asset_getCreatureSpriteSheet(), asset_getCreateSprite(creatureID), colour);

    int score = creature_getScore(i);
    if (score > 0.0f) {
      Vector2 pos         = Vector2Add(POS_ADJUST(creature_getPos(i)), CREATURE_SCORE_OFFSET);
      CREATURE_SCORE.xPos = pos.x + draw_getTextOffset(score);
      CREATURE_SCORE.yPos = pos.y;
      draw_text(CREATURE_SCORE, score);
    }
  }
}

void draw_cursor(void) {
  engine_Sprite* sprite = asset_getCursorSprite();
  // Hack as we want the sprite scaled but the position is also scaled
  int     scale = engine_getScale();
  Vector2 pos   = Vector2Scale(engine_getMousePosition(), 1.0f / scale);
  engine_spriteSetPos(sprite, pos);
  engine_drawSprite(asset_getCursorSpriteSheet(), sprite, WHITE);
}

void draw_interface(void) {
  draw_text(SCORE_TEXT, player_getScore());
  draw_text(LEVEL_TEXT, game_getLevel() + 1, LEVEL_COUNT);
  draw_text(DIFFICULTY_TEXT[game_getDifficulty()]);
}

void draw_nextLife(void) {
  engine_drawSprite(asset_getPlayerSpriteSheet(), asset_getPlayerNextLifeSprite(), WHITE);
  draw_text(EXTRA_LIFE_TEXT, player_getNextExtraLifeScore());
}

void draw_title(void) { draw_text(TITLE_TEXT); }

void draw_ready(void) {
  draw_shadowText(PLAYER_READY_TEXT[game_getDifficulty()]);
  draw_shadowText(SPACE_TEXT);
}

void draw_levelClear(void) {
  engine_drawRectangle(LEVEL_CLEAR_BG_RECTANGLE, LEVEL_CLEAR_BG_COLOUR);
  engine_drawRectangleOutline(LEVEL_CLEAR_BG_RECTANGLE, LEVEL_CLEAR_BG_BORDER);

  draw_shadowText(LEVEL_CLEAR_TEXT);

  player_levelData data = player_getLevelData();
  draw_shadowText(LEVEL_TIME_TEXT, scores_printTime(data.time));
  draw_shadowText(LEVEL_SCORE_TEXT, data.score);

  if (data.clearResult.isTimeRecord) draw_shadowText(TIME_RECORD_TEXT);
  if (data.clearResult.isScoreRecord) draw_shadowText(SCORE_RECORD_TEXT);

  draw_shadowText(SPACE_TEXT);
}

void draw_gameOver(void) {
  draw_shadowText(GAME_OVER_TEXT);
  draw_shadowText(SPACE_TEXT);
}

void draw_gameWon(void) {
  engine_drawRectangle(GAME_WON_BG_RECTANGLE, GAME_WON_BG_COLOUR);
  engine_drawRectangleOutline(GAME_WON_BG_RECTANGLE, GAME_WON_BG_BORDER);

  draw_shadowText(GAME_WON_TEXT);

  player_levelData data = player_getFullRunData();
  draw_shadowText(TOTAL_RUN_TIME_TEXT, scores_printTime(data.time));
  draw_shadowText(TOTal_RUN_SCORE_TEXT, data.score);

  if (data.clearResult.isTimeRecord) draw_shadowText(TOTAL_RUN_TIME_RECORD);
  if (data.clearResult.isScoreRecord) draw_shadowText(TOTAl_RUN_SCORE_RECORD);
}
