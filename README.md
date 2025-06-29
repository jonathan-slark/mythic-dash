# Mythic Dash

## Gameplay

* 3 lives
* +1 life at 10,000, 20,000, 30,000 - display next goal
* Difficulty ramps by ghost speed, AI aggression and power pellet duration
* Ghosts switch between chase and scatter phases
* Ghosts only reverse on mode change
* Scatter phase ghosts retreat to corners
* Ghosts have personalities
* Level 1-4: ghosts get a bit faster

```c
float player_speed = 1.0f;
float ghost_speed = fminf(0.75f + level * 0.02f, 0.95f);
```

* Level 5-10: less scatter, more chase
* Level 11+: minimal scatter, ghosts hit max speed (~90% of player speed)
* Power pellet gradually loses duration
* Cap at 16 levels
* Reduce player speed when eating dots
* Practice mode
* Save states for continuing game

## Pac-man Ghosts

| Ghost      | Nickname | Colour | Chase Logic                                                                                                 |
| ---------- | -------- | ------ | ----------------------------------------------------------------------------------------------------------- |
| **Blinky** | Shadow   | Red    | Directly targets Pac-Man’s **current tile**.                                                                |
| **Pinky**  | Speedy   | Pink   | Targets **four tiles ahead** of Pac-Man, based on his current direction.                                    |
| **Inky**   | Bashful  | Cyan   | Uses a vector based on **both** Blinky’s position **and** four tiles ahead of Pac-Man — very unpredictable. |
| **Clyde**  | Pokey    | Orange | Chases Pac-Man until close, then retreats to a corner — kind of derpy on purpose.                           |

### Ghost Personalities

```c
// pseudocode
vec2 ghost_target_tile(Ghost *ghost, Player *player) {
    switch (ghost->type) {
        case BLINKY: return player->tile;
        case PINKY:  return tile_ahead(player, 4);
        case INKY:   return vector_between(blinky->tile, tile_ahead(player, 2));
        case CLYDE:  return dist(ghost, player) < 8 ? ghost->corner_tile : player->tile;
    }
}
```

### Greedy Direction Selection

```c
// Pseudocode
Direction choose_direction(Ghost *ghost, Maze *maze, Tile target_tile) {
    Direction best_dir = NONE;
    int min_distance = INT_MAX;

    for (Direction dir : all_directions) {
        if (is_wall(maze, ghost->tile, dir)) continue;
        if (dir == opposite_of(ghost->last_direction) && !ghost->can_reverse) continue;

        Tile next_tile = move_tile(ghost->tile, dir);
        int dist = manhattan_distance(next_tile, target_tile);

        if (dist < min_distance || (dist == min_distance && dir < best_dir)) {
            best_dir = dir;
            min_distance = dist;
        }
    }

    return best_dir;
}
```

## Asset Credits

* [Cursive2 font](https://opengameart.org/content/new-original-grafx2-font-collection) - CC0
* [Puny Characters](https://merchant-shade.itch.io/16x16-puny-characters) - CC0
* [Philippine Mythological Creature Sprites](https://merchant-shade.itch.io/ph-myth-creatures) - CC0
* [Assorted RPG Icons](https://merchant-shade.itch.io/16x16-mixed-rpg-icons) - CC0
* Coin by Jonathan Slark - CC0
* [Puny Dungeon](https://merchant-shade.itch.io/16x16-puny-dungeon) - CC0

## Bugs

Assertion failed: bestTileCount > 0 && bestTileCount < startCount, file C:/dev/mythic-dash/src/game/ghost/ghost.c, line 169
