# Player Movement Enhancement Plan

## Current Issues

1. **Invalid Direction Handling**: When the player attempts to move in a direction blocked by a wall, the character stops instead of continuing in the previous valid direction.
2. **Direction Change Queuing**: When holding a key down, the player needs to automatically change direction once reaching a point where that direction becomes valid.

## Implementation Plan

### 1. Movement State Tracking

- Store the current movement direction
- Store the requested movement direction (when key is held down)

```c
typedef enum {
    DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_RIGHT,
    DIRECTION_DOWN,
    DIRECTION_LEFT
} Direction;

typedef struct {
    Direction currentDirection;  // Currently moving direction
    Direction requestedDirection;  // Direction requested by held key
    bool keyIsHeld;              // Whether a movement key is held
} MovementState;
```

### 2. Movement Logic Enhancements

#### Continuous Movement Algorithm

```c
function updateMovement():
    // Try to move in the requested direction if key is held
    if (keyIsHeld && requestedDirection != DIRECTION_NONE):
        if (canMove(playerPosition, requestedDirection)):
            currentDirection = requestedDirection
    
    // Continue moving in current direction if possible
    if (canMove(playerPosition, currentDirection)):
        movePlayer(currentDirection)
    else:
        // Player hit wall, stop movement in this direction
        currentDirection = DIRECTION_NONE
```

#### Input Handling

```c
function onKeyDown(key):
    switch (key):
        case KEY_UP:
            requestedDirection = DIRECTION_UP
            keyIsHeld = true
            break
        case KEY_RIGHT:
            requestedDirection = DIRECTION_RIGHT
            keyIsHeld = true
            break
        // Handle other directions...

function onKeyUp(key):
    if (isMovementKey(key)):
        keyIsHeld = false
        if (no other movement keys are pressed):
            requestedDirection = DIRECTION_NONE
```

### 3. Collision Detection

```c
function canMove(position, direction):
    nextPosition = calculateNextPosition(position, direction)
    return !isWallAt(nextPosition)
    
function calculateNextPosition(position, direction):
    switch (direction):
        case DIRECTION_UP:
            return {position.x, position.y - 1}
        case DIRECTION_RIGHT:
            return {position.x + 1, position.y}
        // Other cases...
```

### 4. Implementation Considerations

1. **Fixed-Point Position Tracking**:
   - Use fixed-point arithmetic for smoother sub-tile movement
   - Only check wall collisions when crossing tile boundaries

2. **Multiple Key Handling**:
   - Track all pressed keys to handle overlapping inputs
   - Prioritize most recently pressed direction when multiple keys are held

3. **Corner Movement**:
   - Implement "corner cutting" by allowing early turns when close to an intersection
   - Use a threshold value to determine how close to a corner a player must be to turn

4. **Movement Timing**:
   - Process movement at a fixed rate independent of rendering frame rate
   - Consider using time-based movement for consistent speed across different hardware

### 5. Testing Scenarios

1. Test continued movement when hitting a wall
2. Test changing direction at intersections with various key presses
3. Test holding a key while approaching an intersection
4. Test rapid key presses at corners and intersections
5. Test diagonal movement attempts (not allowed in maze games)

### 6. Future Enhancements

1. Consider adding a "buffer" to store the last key press for a short time to make corner turns feel more responsive
2. Add visual indicators for requested direction when it's not yet possible to turn
3. Implement smooth interpolation between tiles for more fluid animation
