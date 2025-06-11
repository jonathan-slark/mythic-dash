# Maze Muncher

```sh
maze-muncher/
├── asset/                # Images, sounds, fonts, etc.
├── src/
│   ├── main.c            # Entry point, handles init/game loop
│   ├── game.c/.h         # Core game logic, update/draw loop
│   │
│   ├── player.c/.h       # Player movement, collision, drawing
│   ├── maze.c/.h         # Maze generation, tiles, collisions
│   ├── ghost.c/.h        # AI for enemies / ghost pathing
│   ├── item.c/.h         # Pickups like pellets, power-ups
│   │
│   ├── assets.c/.h       # Texture/font/sound loading
│   ├── state.c/.h        # Game state management (title, play, game over)
│   ├── ui.c/.h           # Score display, menus
│   │
│   └── common.h          # Shared types, constants, macros
```

## Abstration layer

```sh
maze_muncher/
├── engine/
│   ├── engine.h
│   ├── window.h
│   ├── renderer.h
│   ├── texture.h
│   ├── bitmap_font.h
│   └── input.h
├── assets/
│   ├── font.png
│   └── tileset.png
├── src/
│   └── main.c
├── Makefile
```

## Asset Credits

[Cursive2 font](https://opengameart.org/content/new-original-grafx2-font-collection) - CC0
[Puny Characters](https://merchant-shade.itch.io/16x16-puny-characters) - CC0
[Philippine Mythological Creature Sprites](https://merchant-shade.itch.io/ph-myth-creatures) - CC0
