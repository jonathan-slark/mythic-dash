// clang-format Language: C
#pragma once

#include <raylib.h>
#include "../internal.h"
#include "engine/engine.h"

// --- Types ---

constexpr int CREATURE_TOTAL = CREATURE_COUNT * LEVEL_COUNT;

typedef struct asset_AnimData {
  int    row;
  int    startCol;
  int    frameCount;
  double frameTime;
} asset_AnimData;

typedef struct asset_ActorData {
  Vector2        size;
  Vector2        offset;
  Vector2        inset;
  bool           loop;
  asset_AnimData animData[DIR_COUNT];
} asset_ActorData;

typedef struct asset_Sound {
  const char* filepath;
  float       volume;
  float       pitch;
} asset_Sound;

typedef struct asset_Music {
  const char* filepath;
  float       volume;
  float       duckedVolume;
} asset_Music;

typedef struct asset_Assets {
  engine_Texture* creatureSpriteSheet;
  engine_Texture* playerSpriteSheet;
  engine_Sprite*  playerSprites[PLAYER_STATE_COUNT];
  engine_Sprite*  playerLivesSprites[PLAYER_MAX_LIVES];
  engine_Sprite*  playerNextLifeSprite;
  engine_Anim*    playerAnim[PLAYER_STATE_COUNT][DIR_COUNT];
  engine_Sprite*  creatureSprites[CREATURE_TOTAL];
  engine_Anim*    creatureAnims[CREATURE_TOTAL][DIR_COUNT];
  engine_Texture* cursorSpriteSheet;
  engine_Sprite*  cursorSprite;
  engine_Font*    font;
  engine_Font*    fontTiny;
  engine_Sound*   wailSounds[WAIL_SOUND_COUNT];
  engine_Sound*   chimeSound;
  engine_Sound*   deathSound;
  engine_Sound*   fallingSound;
  engine_Sound*   whispersSound;
  engine_Sound*   pickupSound;
  engine_Sound*   twinkleSound;
  engine_Sound*   winSound;
  engine_Sound*   gameOverSound;
  engine_Sound*   lifeSound;
  engine_Music*   music[MUSIC_TRACKS];
} asset_Assets;

// --- Constants ---

static const char FILE_BACKGROUND[] = ASSET_DIR "gfx/background.png";
static const char FILE_CREATURES[]  = ASSET_DIR "gfx/creatures.png";
static const char FILE_PLAYER[]     = ASSET_DIR "gfx/player.png";
static const char FILE_FONT[]       = ASSET_DIR "gfx/font.png";
static const char FILE_FONT_TINY[]  = ASSET_DIR "gfx/tiny-numbers.png";

static const asset_Sound WAIL_SOUNDS[] = {
  { .filepath = ASSET_DIR "sfx/wail-down.wav", .volume = 0.2f, .pitch = 1.0f },
  { .filepath = ASSET_DIR "sfx/wail-down.wav", .volume = 0.2f, .pitch = 0.8f },
  {   .filepath = ASSET_DIR "sfx/wail-up.wav", .volume = 0.2f, .pitch = 1.0f },
  {   .filepath = ASSET_DIR "sfx/wail-up.wav", .volume = 0.2f, .pitch = 1.2f }
};
static const asset_Sound CHIME_SOUND     = { .filepath = ASSET_DIR "sfx/chime.wav", .volume = 0.5f, .pitch = 1.0f };
static const asset_Sound DEATH_SOUND     = { .filepath = ASSET_DIR "sfx/death.wav", .volume = 1.0f, .pitch = 1.0f };
static const asset_Sound FALLING_SOUND   = { .filepath = ASSET_DIR "sfx/falling.wav", .volume = 1.0f, .pitch = 1.0f };
static const asset_Sound WHISPERS_SOUND  = { .filepath = ASSET_DIR "sfx/whispers.wav", .volume = 1.0f, .pitch = 1.0f };
static const asset_Sound PICKUP_SOUND    = { .filepath = ASSET_DIR "sfx/pickup.wav", .volume = 1.0f, .pitch = 1.0f };
static const asset_Sound TWINKLE_SOUND   = { .filepath = ASSET_DIR "sfx/twinkle.mp3", .volume = 1.0f, .pitch = 1.0f };
static const asset_Sound WIN_SOUND       = { .filepath = ASSET_DIR "sfx/win.wav", .volume = 1.0f, .pitch = 1.0f };
static const asset_Sound GAME_OVER_SOUND = { .filepath = ASSET_DIR "sfx/game-over.wav", .volume = 1.0f, .pitch = 1.0f };
static const asset_Sound LIFE_SOUND      = { .filepath = ASSET_DIR "sfx/life.wav", .volume = 1.0f, .pitch = 1.0f };

static const float       FADE_IN_RATE        = 1.0f / 1.0f;
static const float       FADE_OUT_RATE       = 1.0f / 0.25f;
static const asset_Music MUSIC[MUSIC_TRACKS] = {
  { .filepath = ASSET_DIR "music/01.wav", .volume = 0.4f, .duckedVolume = 0.2f },
  { .filepath = ASSET_DIR "music/02.wav", .volume = 0.4f, .duckedVolume = 0.2f },
  { .filepath = ASSET_DIR "music/03.wav", .volume = 0.4f, .duckedVolume = 0.2f },
  { .filepath = ASSET_DIR "music/04.wav", .volume = 0.4f, .duckedVolume = 0.2f },
  { .filepath = ASSET_DIR "music/05.wav", .volume = 0.4f, .duckedVolume = 0.2f },
  { .filepath = ASSET_DIR "music/06.wav", .volume = 0.4f, .duckedVolume = 0.2f },
  { .filepath = ASSET_DIR "music/07.wav", .volume = 0.4f, .duckedVolume = 0.2f }
};

static const float ANIM_FRAME_TIME = 0.1f;

static const Vector2 PLAYER_LIVES_OFFSET     = { 8.0f, 248.0f };
static const Vector2 PLAYER_NEXT_LIFE_OFFSET = { 412.0f, 248.0f };

static const Vector2 CURSOR_SIZE = { 16.0f, 16.0f };
static const int     CURSOR_ROW  = 18;
static const int     CURSOR_COL  = 25;

// clang-format off
static const asset_ActorData PLAYER_DATA[PLAYER_STATE_COUNT] = {
  {
    .inset    = { 8.0f, 8.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 4, .startCol = 0, .frameCount = 5, ANIM_FRAME_TIME},
      [DIR_RIGHT] = { .row = 2, .startCol = 0, .frameCount = 5, ANIM_FRAME_TIME},
      [DIR_DOWN]  = { .row = 0, .startCol = 0, .frameCount = 5, ANIM_FRAME_TIME},
      [DIR_LEFT]  = { .row = 6, .startCol = 0, .frameCount = 5, ANIM_FRAME_TIME}
    }
  },
  {
    .inset    = { 8.0f, 8.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 4, .startCol = 5, .frameCount = 4, ANIM_FRAME_TIME},
      [DIR_RIGHT] = { .row = 2, .startCol = 5, .frameCount = 4, ANIM_FRAME_TIME},
      [DIR_DOWN]  = { .row = 0, .startCol = 5, .frameCount = 4, ANIM_FRAME_TIME},
      [DIR_LEFT]  = { .row = 6, .startCol = 5, .frameCount = 4, ANIM_FRAME_TIME}
    }
  },
  {
    .inset    = { 8.0f, 8.0f },
    .loop = false,
    .animData = {
      [DIR_UP]    = { .row = 4, .startCol = 21, .frameCount = 3, ANIM_FRAME_TIME * 2.0f},
      [DIR_RIGHT] = { .row = 2, .startCol = 21, .frameCount = 3, ANIM_FRAME_TIME * 2.0f},
      [DIR_DOWN]  = { .row = 0, .startCol = 21, .frameCount = 3, ANIM_FRAME_TIME * 2.0f},
      [DIR_LEFT]  = { .row = 6, .startCol = 21, .frameCount = 3, ANIM_FRAME_TIME * 2.0f}
    }
  },
  {
    .inset    = { 8.0f, 8.0f },
    .loop = false,
    .animData = {
      [DIR_UP]    = { .row = 10, .startCol = 0, .frameCount = 5, ANIM_FRAME_TIME},
      [DIR_RIGHT] = { .row = 9, .startCol = 0, .frameCount = 5, ANIM_FRAME_TIME},
      [DIR_DOWN]  = { .row = 8, .startCol = 0, .frameCount = 5, ANIM_FRAME_TIME},
      [DIR_LEFT]  = { .row = 11, .startCol = 0, .frameCount = 5, ANIM_FRAME_TIME}
    }
  }  
};

static const asset_ActorData CREATURE_DATA[CREATURE_TOTAL] = {
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f  },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 3, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 2, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 0, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 1, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f  },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 7, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 6, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 4, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 5, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 7, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 6, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 4, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 5, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 7, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 6, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 4, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 5, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 7, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 6, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 4, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 5, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 7, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 6, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 4, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 5, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 7, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 6, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 4, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 5, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f  },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 11, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 10, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 8, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 9, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 11, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 10, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 8, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 9, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 11, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 10, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 8, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 9, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 11, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 10, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 8, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 9, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 11, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 10, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 8, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 9, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 11, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 10, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 8, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 9, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f  },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 15, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 14, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 12, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 13, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 15, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 14, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 12, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 13, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 15, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 14, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 12, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 13, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 15, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 14, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 12, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 13, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 15, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 14, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 12, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 13, .startCol = 12, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 15, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 14, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 12, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 13, .startCol = 15, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f  },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 19, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 18, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 16, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 17, .startCol = 0, .frameCount = 3, ANIM_FRAME_TIME }
    }
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,  0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 19, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 18, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 16, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 17, .startCol = 3, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 19, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 18, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 16, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 17, .startCol = 6, .frameCount = 3, ANIM_FRAME_TIME }
    },
  },
  {
    .size     = { 24.0f, 24.0f },
    .offset   = { -4.0f, -8.0f },
    .inset    = { 0.0f,   0.0f },
    .loop = true,
    .animData = {
      [DIR_UP]    = { .row = 19, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_RIGHT] = { .row = 18, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_DOWN]  = { .row = 16, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME },
      [DIR_LEFT]  = { .row = 17, .startCol = 9, .frameCount = 3, ANIM_FRAME_TIME }
    }
  }
};
// clang-format on
