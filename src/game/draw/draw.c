#include "draw.h"
#include "../asset/asset.h"
#include "../ghost/ghost.h"
#include "../internal.h"
#include "../player/player.h"

// --- Constants ---

const char*          PLAYER_STATE_STRINGS[PLAYER_STATE_COUNT] = { "NORMAL", "SWORD", "DEAD" };
static const Color   GHOST_DEAD_COLOUR                        = { 255, 255, 255, 100 };
static const Vector2 PLAYER_COOLDOWN_OFFSET                   = { 5, -8 };
static const Vector2 GHOST_SCORE_OFFSET                       = { 1, -8 };

// --- Draw functions ---

void draw_updatePlayer(float frameTime, float slop) {
  player_update(frameTime, slop);

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

void draw_updateGhosts(float frameTime, float slop) {
  ghost_update(frameTime, slop);
  for (int i = 0; i < CREATURE_COUNT; i++) {
    Vector2 pos = Vector2Add(POS_ADJUST(ghost_getPos(i)), asset_getCreatureOffset(i));
    engine_spriteSetPos(asset_getCreateSprite(i), pos);
    engine_updateAnim(asset_getCreatureAnim(i, ghost_getDir(i)), frameTime);
  }
}

void draw_ghosts(void) {
  for (int i = 0; i < CREATURE_COUNT; i++) {
    Color colour = ghost_isFrightened(i) ? BLUE : ghost_isDead(i) ? GHOST_DEAD_COLOUR : WHITE;
    engine_drawSprite(asset_getCreatureSpriteSheet(), asset_getCreateSprite(i), colour);

    int score = ghost_getScore(i);
    if (score > 0.0f) {
      Vector2 pos = Vector2Add(POS_ADJUST(ghost_getPos(i)), GHOST_SCORE_OFFSET);
      engine_fontPrintf(asset_getFontTiny(), pos.x, pos.y, WHITE, "%d", score);
    }
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
    Vector2 pos = Vector2Add(POS_ADJUST(player_getPos()), PLAYER_COOLDOWN_OFFSET);
    engine_fontPrintf(asset_getFontTiny(), pos.x, pos.y, WHITE, "%d", (int) ceilf(swordTimer), pos);
  }
}

void draw_interface(void) {
  engine_fontPrintf(asset_getFont(), 8, 0, WHITE, "Score: %d", player_getScore());
  engine_fontPrintf(asset_getFont(), 394, 0, WHITE, "Level: %02d / ?", game_getLevel());
}
