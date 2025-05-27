# Game Subsystem Unit Tests

This directory contains unit tests for the maze-muncher game subsystem using the MinUnit testing framework.

## Test Coverage

The test suite provides basic coverage for:

### AABB (Axis-Aligned Bounding Box) System

- Collision detection between rectangular areas
- Overlap calculation for X and Y axes
- Edge cases (touching boundaries, no overlap)

### Maze System  

- Wall/floor detection using simplified maze data
- AABB generation for maze tiles
- Boundary validation and out-of-bounds handling

### Game Constants

- Direction enumeration validation
- Core game constants verification (tile size, actor size, slop values)
- Maze origin coordinates

### Expected Output

The test runner will display progress indicators (dots for passing tests, 'F' for failures) and provide a summary including:

- Total number of tests run
- Number of assertions executed  
- Number of failures (if any)
- Execution time

## Test Structure

Tests are organized into suites:

- `aabb_suite`: Tests for collision detection and geometric calculations
- `maze_suite`: Tests for maze structure and navigation
- `constants_suite`: Tests for game configuration values
- `game_logic_suite`: Tests for core game mechanics

## Notes

- This test suite uses isolated implementations to avoid external dependencies
- The maze data is simplified for testing purposes
- Tests focus on pure algorithmic functionality rather than graphics/input
- Uses modern C23 features including `constexpr` and designated initializers

## Adding New Tests

To add new tests:

1. Define test functions using `MU_TEST(test_name)`
2. Add test to appropriate suite using `MU_RUN_TEST(test_name)`
3. Create new suites if needed using `MU_TEST_SUITE(suite_name)`
4. Add suite to main runner using `MU_RUN_SUITE(suite_name)`
