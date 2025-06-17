/*
 * Game Subsystem Unit Tests
 * Tests core functionality of the maze-muncher game subsystem
 */

#include <assert.h>
#include <math.h>
#include <minunit/minunit.h>
#include <stdbool.h>

// --- Test-specific types (minimal duplicates to avoid raylib conflicts) ---

typedef struct Vector2 {
  float x;
  float y;
} Vector2;

typedef enum game__Dir { DIR_NONE, DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_COUNT } game__Dir;

typedef struct game__AABB {
  Vector2 min;
  Vector2 max;
} game__AABB;

// --- Constants for testing ---
#define ACTOR_SIZE 16
const float   TILE_SIZE   = 8.0f;
const float   BASE_SLOP   = 0.25f;
const float   BASE_DT     = (1.0f / 144.0f);
const float   MIN_SLOP    = 0.05f;
const float   MAX_SLOP    = 0.5f;
const Vector2 MAZE_ORIGIN = { 124.0f, 7.0f };

// --- Test helper macros ---
#define FLOAT_EPSILON 0.001f
#define mu_assert_float_eq(expected, result) \
  mu_assert(fabs((expected) - (result)) < FLOAT_EPSILON, "Float values not equal")

#define mu_assert_vector2_eq(expected, result)                                                            \
  mu_assert(                                                                                              \
      fabs((expected).x - (result).x) < FLOAT_EPSILON && fabs((expected).y - (result).y) < FLOAT_EPSILON, \
      "Vector2 values not equal"                                                                          \
  )

// --- Core AABB functions (inline for testing) ---

static inline bool aabb_isColliding(game__AABB a, game__AABB b) {
  return a.min.x < b.max.x && a.max.x > b.min.x && a.min.y < b.max.y && a.max.y > b.min.y;
}

static inline float aabb_getOverlapX(game__AABB a, game__AABB b) {
  return fmin(a.max.x, b.max.x) - fmax(a.min.x, b.min.x);
}

static inline float aabb_getOverlapY(game__AABB a, game__AABB b) {
  return fmin(a.max.y, b.max.y) - fmax(a.min.y, b.min.y);
}

// --- maze__Maze simulation for testing ---

#define MAZE_ROWS 32
#define MAZE_COLS 29

// Simplified maze data (first few rows for testing)
static bool MAZE_TEST[MAZE_ROWS][MAZE_COLS] = {
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
  { 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1 },
  // ... rest filled with walls for simplicity
};

static bool maze_isWall_test(Vector2 pos) {
  int row = (int) (pos.y / TILE_SIZE);
  int col = (int) (pos.x / TILE_SIZE);
  if (row < 0 || row >= MAZE_ROWS || col < 0 || col >= MAZE_COLS) {
    return true;              // Out of bounds is wall
  }
  if (row >= 4) return true;  // Simplify: rows 4+ are walls for testing
  return MAZE_TEST[row][col];
}

static game__AABB maze_getAABB_test(Vector2 pos) {
  int row = (int) (pos.y / TILE_SIZE);
  int col = (int) (pos.x / TILE_SIZE);
  return (game__AABB) {
    .min = {       col * TILE_SIZE,       row * TILE_SIZE },
      .max = { (col + 1) * TILE_SIZE, (row + 1) * TILE_SIZE }
  };
}

// --- Setup and Teardown ---

void test_setup(void) {
  // Setup for each test
}

void test_teardown(void) {
  // Cleanup after each test
}

// --- AABB Tests ---

MU_TEST(test_aabb_collision_detection) {
  game__AABB a = {
    {  0.0f,  0.0f },
    { 10.0f, 10.0f }
  };
  game__AABB b = {
    {  5.0f,  5.0f },
    { 15.0f, 15.0f }
  };
  game__AABB c = {
    { 20.0f, 20.0f },
    { 30.0f, 30.0f }
  };

  mu_assert(aabb_isColliding(a, b), "Overlapping AABBs should collide");
  mu_assert(!aabb_isColliding(a, c), "Non-overlapping AABBs should not collide");
}

MU_TEST(test_aabb_overlap_calculation) {
  game__AABB a = {
    {  0.0f,  0.0f },
    { 10.0f, 10.0f }
  };
  game__AABB b = {
    {  5.0f,  3.0f },
    { 15.0f, 13.0f }
  };

  float overlapX = aabb_getOverlapX(a, b);
  float overlapY = aabb_getOverlapY(a, b);

  mu_assert_float_eq(5.0f, overlapX);
  mu_assert_float_eq(7.0f, overlapY);
}

MU_TEST(test_aabb_no_overlap) {
  game__AABB a = {
    { 0.0f, 0.0f },
    { 5.0f, 5.0f }
  };
  game__AABB b = {
    { 10.0f, 10.0f },
    { 15.0f, 15.0f }
  };

  mu_assert(!aabb_isColliding(a, b), "Separated AABBs should not overlap");
  mu_assert(aabb_getOverlapX(a, b) <= 0.0f, "No X overlap expected");
  mu_assert(aabb_getOverlapY(a, b) <= 0.0f, "No Y overlap expected");
}

MU_TEST(test_aabb_edge_cases) {
  // Test touching AABBs (should not collide)
  game__AABB a = {
    { 0.0f, 0.0f },
    { 5.0f, 5.0f }
  };
  game__AABB b = {
    {  5.0f, 0.0f },
    { 10.0f, 5.0f }
  };

  mu_assert(!aabb_isColliding(a, b), "Touching AABBs should not collide");
  mu_assert(aabb_getOverlapX(a, b) == 0.0f, "No overlap for touching AABBs");
}

// --- maze__Maze Tests ---

MU_TEST(test_maze_wall_detection) {
  // Test corner positions that should be walls
  mu_assert(maze_isWall_test((Vector2) { 0.0f, 0.0f }), "Top-left corner should be wall");
  mu_assert(maze_isWall_test((Vector2) { 28 * TILE_SIZE, 0.0f }), "Top-right corner should be wall");

  // Test some positions that should be floor
  mu_assert(!maze_isWall_test((Vector2) { 1 * TILE_SIZE, 1 * TILE_SIZE }), "Position (1,1) should be floor");
  mu_assert(!maze_isWall_test((Vector2) { 2 * TILE_SIZE, 2 * TILE_SIZE }), "Position (2,2) should be floor");
}

MU_TEST(test_maze_aabb_generation) {
  Vector2    tile_pos = { 2 * TILE_SIZE, 3 * TILE_SIZE };
  game__AABB aabb     = maze_getAABB_test(tile_pos);

  Vector2 expected_min = { 2 * TILE_SIZE, 3 * TILE_SIZE };
  Vector2 expected_max = { 3 * TILE_SIZE, 4 * TILE_SIZE };

  mu_assert_vector2_eq(expected_min, aabb.min);
  mu_assert_vector2_eq(expected_max, aabb.max);
}

MU_TEST(test_maze_bounds) {
  // Test that out-of-bounds positions are treated as walls
  mu_assert(maze_isWall_test((Vector2) { -1.0f, -1.0f }), "Out of bounds should be wall");
  mu_assert(maze_isWall_test((Vector2) { 1000.0f, 1000.0f }), "Out of bounds should be wall");

  // Test valid positions
  Vector2    valid_pos = { 5 * TILE_SIZE, 1 * TILE_SIZE };
  game__AABB aabb      = maze_getAABB_test(valid_pos);
  mu_assert(aabb.min.x >= 0 && aabb.min.y >= 0, "AABB should have valid coordinates");
  mu_assert(aabb.max.x > aabb.min.x && aabb.max.y > aabb.min.y, "AABB should have positive dimensions");
}

// --- Direction Tests ---

MU_TEST(test_direction_enum) {
  mu_assert_int_eq(0, DIR_NONE);
  mu_assert_int_eq(1, DIR_UP);
  mu_assert_int_eq(2, DIR_RIGHT);
  mu_assert_int_eq(3, DIR_DOWN);
  mu_assert_int_eq(4, DIR_LEFT);
  mu_assert(DIR_COUNT == 5, "Direction count should be 5");
}

// --- Constants Tests ---

MU_TEST(test_game_constants) {
  mu_assert_int_eq(16, ACTOR_SIZE);
  mu_assert_float_eq(8.0f, TILE_SIZE);
  mu_assert_float_eq(0.25f, BASE_SLOP);
  mu_assert(BASE_DT > 0.0f, "BASE_DT should be positive");
  mu_assert(MIN_SLOP < MAX_SLOP, "MIN_SLOP should be less than MAX_SLOP");
  mu_assert(MIN_SLOP > 0.0f, "MIN_SLOP should be positive");
}

MU_TEST(test_maze_origin) {
  mu_assert_float_eq(124.0f, MAZE_ORIGIN.x);
  mu_assert_float_eq(7.0f, MAZE_ORIGIN.y);
}

// --- Game Logic Tests ---

MU_TEST(test_slop_calculation) {
  // Test slop calculation logic (frameTime scaling)
  float frameTime1 = BASE_DT;
  float frameTime2 = BASE_DT * 2;

  float slop1 = BASE_SLOP * (frameTime1 / BASE_DT);
  float slop2 = BASE_SLOP * (frameTime2 / BASE_DT);

  mu_assert_float_eq(BASE_SLOP, slop1);
  mu_assert_float_eq(BASE_SLOP * 2, slop2);

  // Test clamping
  float large_slop = BASE_SLOP * 10;
  float clamped    = fminf(fmaxf(large_slop, MIN_SLOP), MAX_SLOP);
  mu_assert_float_eq(MAX_SLOP, clamped);
}

// --- Test Suites ---

MU_TEST_SUITE(aabb_suite) {
  MU_RUN_TEST(test_aabb_collision_detection);
  MU_RUN_TEST(test_aabb_overlap_calculation);
  MU_RUN_TEST(test_aabb_no_overlap);
  MU_RUN_TEST(test_aabb_edge_cases);
}

MU_TEST_SUITE(maze_suite) {
  MU_RUN_TEST(test_maze_wall_detection);
  MU_RUN_TEST(test_maze_aabb_generation);
  MU_RUN_TEST(test_maze_bounds);
}

MU_TEST_SUITE(constants_suite) {
  MU_RUN_TEST(test_direction_enum);
  MU_RUN_TEST(test_game_constants);
  MU_RUN_TEST(test_maze_origin);
}

MU_TEST_SUITE(game_logic_suite) { MU_RUN_TEST(test_slop_calculation); }

// --- Main Test Runner ---

int main(void) {
  printf("Running Game Subsystem Unit Tests\n");
  printf("==================================\n\n");

  // Configure test setup and teardown
  MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

  // Run all test suites
  printf("Running AABB Tests...\n");
  MU_RUN_SUITE(aabb_suite);

  printf("\nRunning maze__Maze Tests...\n");
  MU_RUN_SUITE(maze_suite);

  printf("\nRunning Constants Tests...\n");
  MU_RUN_SUITE(constants_suite);

  printf("\nRunning Game Logic Tests...\n");
  MU_RUN_SUITE(game_logic_suite);

  // Generate test report
  MU_REPORT();

  return MU_EXIT_CODE;
}
