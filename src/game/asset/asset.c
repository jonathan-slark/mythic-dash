#include "asset.h"
#include <assert.h>
#include <engine/engine.h>
#include <raylib.h>
#include "../creature/creature.h"
#include "../internal.h"
#include "../player/player.h"
#include "internal.h"

// --- Constants ---

static const int MAX_SOUNDS = 10;

// --- Global state ---

static asset_Assets g_assets;

// --- Helper functions

bool loadSound(asset_Sound soundData, engine_Sound** sound) {
  GAME_TRY(*sound = engine_loadSound(soundData.filepath, MAX_SOUNDS));
  engine_setSoundVolume(*sound, soundData.volume);
  engine_setSoundPitch(*sound, soundData.pitch);
  return true;
}

// --- Asset functions ---

bool asset_load(void) {
  GAME_TRY(g_assets.creatureSpriteSheet = engine_textureLoad(FILE_CREATURES));
  GAME_TRY(g_assets.playerSpriteSheet = engine_textureLoad(FILE_PLAYER));
  GAME_TRY(g_assets.font = engine_fontLoad(FILE_FONT, 6, 10, 32, 127, 0, 2));
  GAME_TRY(g_assets.fontTiny = engine_fontLoad(FILE_FONT_TINY, 5, 7, 48, 57, 0, 0));
  for (int i = 0; i < WAIL_SOUND_COUNT; i++) {
    GAME_TRY(loadSound(WAIL_SOUNDS[i], &g_assets.wailSounds[i]));
  }
  GAME_TRY(loadSound(CHIME_SOUND, &g_assets.chimeSound));
  GAME_TRY(loadSound(DEATH_SOUND, &g_assets.deathSound));
  return true;
}

void asset_unload(void) {
  engine_unloadSound(g_assets.chimeSound);
  engine_fontUnload(&g_assets.font);
  engine_textureUnload(&g_assets.playerSpriteSheet);
  engine_textureUnload(&g_assets.creatureSpriteSheet);
}

bool asset_initPlayer(void) {
  GAME_TRY(player_init());

  for (int i = 0; i < PLAYER_STATE_COUNT; i++) {
    GAME_TRY(
        g_assets.playerSprites[i] = engine_createSprite(
            POS_ADJUST(player_getPos()), (Vector2) { ACTOR_SIZE, ACTOR_SIZE }, (Vector2) { 0.0f, 0.0f }
        )
    );
    for (int j = 0; j < DIR_COUNT; j++) {
      GAME_TRY(
          g_assets.playerAnim[i][j] = engine_createAnim(
              g_assets.playerSprites[i],
              PLAYER_DATA[i].animData[j].row,
              PLAYER_DATA[i].animData[j].startCol,
              PLAYER_DATA[i].animData[j].frameCount,
              PLAYER_DATA[i].animData[j].frameTime,
              PLAYER_DATA[i].inset,
              PLAYER_DATA[i].loop
          )
      );
    }
  }

  Vector2 offset = PLAYER_LIVES_OFFSET;
  for (int i = 0; i < PLAYER_MAX_LIVES; i++) {
    GAME_TRY(
        g_assets.playerLivesSprites[i] = engine_createSpriteFromSheet(
            offset,
            (Vector2) { ACTOR_SIZE, ACTOR_SIZE },
            PLAYER_DATA[PLAYER_NORMAL].animData[DIR_LEFT].row,
            PLAYER_DATA[PLAYER_NORMAL].animData[DIR_LEFT].startCol,
            PLAYER_DATA[PLAYER_NORMAL].inset
        )
    );
    offset.x += ACTOR_SIZE;
  }

  GAME_TRY(
      g_assets.playerNextLifeSprite = engine_createSpriteFromSheet(
          PLAYER_NEXT_LIFE_OFFSET,
          (Vector2) { ACTOR_SIZE, ACTOR_SIZE },
          PLAYER_DATA[PLAYER_NORMAL].animData[DIR_LEFT].row,
          PLAYER_DATA[PLAYER_NORMAL].animData[DIR_LEFT].startCol,
          PLAYER_DATA[PLAYER_NORMAL].inset
      )
  );

  return true;
}

bool asset_initCreatures(void) {
  GAME_TRY(creature_init());
  for (int i = 0; i < CREATURE_COUNT; i++) {
    GAME_TRY(
        g_assets.creatureSprites[i] =
            engine_createSprite(POS_ADJUST(creature_getPos(i)), CREATURE_DATA[i].size, (Vector2) { 0.0f, 0.0f })
    );
    for (int j = 0; j < DIR_COUNT; j++) {
      GAME_TRY(
          g_assets.creatureAnims[i][j] = engine_createAnim(
              g_assets.creatureSprites[i],
              CREATURE_DATA[i].animData[j].row,
              CREATURE_DATA[i].animData[j].startCol,
              CREATURE_DATA[i].animData[j].frameCount,
              CREATURE_DATA[i].animData[j].frameTime,
              CREATURE_DATA[i].inset,
              CREATURE_DATA[i].loop
          )
      );
    }
  }
  return true;
}

void asset_shutdownPlayer(void) {
  player_shutdown();
  for (int i = 0; i < PLAYER_LIVES; i++) {
    engine_destroySprite(&g_assets.playerLivesSprites[i]);
  }
  for (int i = 0; i < PLAYER_STATE_COUNT; i++) {
    engine_destroySprite(&g_assets.playerSprites[i]);
    for (int j = 0; j < DIR_COUNT; j++) {
      engine_destroyAnim(&g_assets.playerAnim[i][j]);
    }
  }
}

void asset_shutdownCreatures(void) {
  creature_shutdown();
  for (int i = 0; i < CREATURE_COUNT; i++) {
    engine_destroySprite(&g_assets.creatureSprites[i]);
    for (int j = 0; j < DIR_COUNT; j++) {
      engine_destroyAnim(&g_assets.creatureAnims[i][j]);
    }
  }
}

engine_Texture* asset_getCreatureSpriteSheet(void) {
  assert(g_assets.creatureSpriteSheet != nullptr);
  return g_assets.creatureSpriteSheet;
}

engine_Texture* asset_getPlayerSpriteSheet(void) {
  assert(g_assets.playerSpriteSheet != nullptr);
  return g_assets.playerSpriteSheet;
}

engine_Sprite* asset_getPlayerSprite(game_PlayerState state) {
  assert(state >= 0 && state < PLAYER_STATE_COUNT);
  return g_assets.playerSprites[state];
}

engine_Sprite* asset_getPlayerLivesSprite(int life) {
  assert(life >= 0 && life < PLAYER_MAX_LIVES);
  return g_assets.playerLivesSprites[life];
}

engine_Sprite* asset_getPlayerNextLifeSprite(void) {
  assert(g_assets.playerNextLifeSprite != nullptr);
  return g_assets.playerNextLifeSprite;
}

engine_Anim* asset_getPlayerAnim(game_PlayerState state, game_Dir dir) {
  assert(state >= 0 && state < PLAYER_STATE_COUNT);
  assert(dir >= 0 && dir < DIR_COUNT);
  return g_assets.playerAnim[state][dir];
}

engine_Sprite* asset_getCreateSprite(int creatureID) {
  assert(creatureID >= 0 && creatureID < CREATURE_COUNT);
  return g_assets.creatureSprites[creatureID];
}

engine_Anim* asset_getCreatureAnim(int creatureID, game_Dir dir) {
  assert(creatureID >= 0 && creatureID < CREATURE_COUNT);
  assert(dir >= 0 && dir < DIR_COUNT);
  return g_assets.creatureAnims[creatureID][dir];
}

Vector2 asset_getCreatureOffset(int creatureID) {
  assert(creatureID >= 0 && creatureID < CREATURE_COUNT);
  return CREATURE_DATA[creatureID].offset;
}

engine_Font* asset_getFont(void) {
  assert(g_assets.font != nullptr);
  return g_assets.font;
}

engine_Font* asset_getFontTiny(void) {
  assert(g_assets.fontTiny != nullptr);
  return g_assets.fontTiny;
}

engine_Sound* asset_getWailSound(int id) {
  assert(id >= 0 && id < WAIL_SOUND_COUNT);
  assert(g_assets.wailSounds[id] != nullptr);
  return g_assets.wailSounds[id];
}

engine_Sound* asset_getChimeSound(void) {
  assert(g_assets.chimeSound != nullptr);
  return g_assets.chimeSound;
}

engine_Sound* asset_getDeathSound(void) {
  assert(g_assets.deathSound != nullptr);
  return g_assets.deathSound;
}
