# Style Guide

## General Philosophy

* Use clear, descriptive names that reflect function or purpose.
* Keep naming flat and modular: avoid ambiguous globals by prefixing with module name.
* Be consistent with case style and prefixing, especially across subsystems.

## Modules & Prefixes

Each subsystem or source directory gets a short prefix:

* game_ → Public game API
* player_, ghost_, maze_, actor_, debug_, etc. → Internal subsystem functions and types

## Function Naming

| Scope        | Format                     | Example                           | Notes                                         |
| ------------ | -------------------------- | --------------------------------- | --------------------------------------------- |
| Public API   | `subsystem_functionName()` | `game_init()`, `game_update()`    | Public game-facing API. Avoid deep internals. |
| Internal     | `module_functionName()`    | `player_update()`, `maze_reset()` | Used within subsystems. Local to module.      |
| Static local | `static void helper()`     | `static void drawScoreBar()`      | Only visible within one .c file.              |

## Type Naming

| Scope        | Format                   | Example                     | Notes                                            |
| ------------ | ------------------------ | --------------------------- | ------------------------------------------------ |
| Shared Types | `subsystem_TypeName`     | `game_AABB`, `game_Dir`     | For types shared across modules.                 |
| Internal     | `module_TypeName`        | `player_State`, `maze_Tile` | Scoped to module. Prefer `PascalCase` for types. |
| Enums        | `unit_EnumName` + CAPS   | `game_Dir`, `PLAYER_DEAD`   | Enum type in PascalCase, values in CAPS.         |

## Files and Includes

* Public headers go in include/subsystem/ — minimal interface.
* Internal headers live in src/module/ — scoped to that module.
* Avoid including .c files or sharing internal headers across unrelated modules.

## Example Usage

```c
// player.h (internal)
typedef struct player_State {
  int lives;
  Vector2 pos;
} player_State;

void player_update(float frameTime);
void player_reset(void);
```

```c
// game.h (public)
bool game_init(void);
void game_update(float frameTime);
void game_shutdown(void);
```
