# Mythic Dash

## Gameplay

* 3 lives
* +1 life at 10,000, 20,000, 30,000 - display next goal
* Difficulty ramps by ghost speed, AI aggression and power pellet duration
* Ghosts switch between chase and scatter phases
* Ghosts have personalities
* Level 1-4: ghosts get a bit faster
* Level 5-10: less scatter, more chase
* Level 11+: minimal scatter, ghosts hit max speed (~90% of player speed)
* Power pellet gradually loses duration
* Cap at 16 levels
* Reduce player speed when eating dots
* Practice mode
* Save states for continuing game

```c
float player_speed = 1.0f;
float ghost_speed = fminf(0.75f + level * 0.02f, 0.95f);
```

## Asset Credits

[Cursive2 font](https://opengameart.org/content/new-original-grafx2-font-collection) - CC0
[Puny Characters](https://merchant-shade.itch.io/16x16-puny-characters) - CC0
[Philippine Mythological Creature Sprites](https://merchant-shade.itch.io/ph-myth-creatures) - CC0
[Puny Dungeon](https://merchant-shade.itch.io/16x16-puny-dungeon) - CC0
