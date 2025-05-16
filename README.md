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
