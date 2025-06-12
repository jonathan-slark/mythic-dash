#include <assert.h>
#include <errno.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "log/log.h"

// --- Macros ---
#define RETURN_FAIL() \
  do {                \
    unload(&maze);    \
    return false;     \
  } while (0)

// --- Constants ---

const Vector2 MAZE_ORIGIN = { 0.0f, 8.0f };  // Screen offset to the actual maze
constexpr int MAZE_ROWS   = 16;
constexpr int MAZE_COLS   = 30;
constexpr int BUFFER_SIZE = 32;
#define OVERLAY_COLOUR_MAZE_WALL (Color){ 128, 128, 128, 128 }

static const char* FILE_MAZE = "../../asset/map/maze01.csv";

// --- Global state ---

static int        g_maze[MAZE_ROWS][MAZE_COLS];
static game__AABB g_mazeAABB[MAZE_ROWS][MAZE_COLS];

// --- Helper functions ---

static void closeCheck(FILE* fp, const char* file) {
  assert(fp != nullptr);
  if (fclose(fp) == EOF) {
    LOG_ERROR(game__log, "Error on closing file: %s (%s)", file, strerror(errno));
  }
}

static char* load(const char* file, const char* mode) {
  FILE* fp = fopen(file, mode);
  if (!fp) {
    LOG_FATAL(game__log, "Unable to open file: %s (%s)", file, strerror(errno));
    return nullptr;
  }

  if (fseek(fp, 0L, SEEK_END) != 0) {
    LOG_FATAL(game__log, "Error on seeking file: %s (%s)", file, strerror(errno));
    closeCheck(fp, file);
    return nullptr;
  }
  long l = ftell(fp);
  if (l < 0) {
    LOG_FATAL(game__log, "Error on getting size of file: %s (%s)", file, strerror(errno));
    closeCheck(fp, file);
    return nullptr;
  }
  size_t size = (size_t) l;
  rewind(fp);

  char* data = (char*) malloc((size + 1) * sizeof(char));
  if (data == nullptr) {
    LOG_FATAL(game__log, "Error allocating memory");
    return nullptr;
  }

  size_t result = fread(data, sizeof(char), size, fp);
  if (result < size) {
    LOG_FATAL(
        game__log, "Error reading file: %s (expected size: %zu, actual: %zu, %s)", file, size, result, strerror(errno)
    );
    free(data);
    closeCheck(fp, file);
    return nullptr;
  }
  data[size] = '\0';

  if (fclose(fp) == EOF) {
    LOG_ERROR(game__log, "Error on closing file: %s (%s)", file, strerror(errno));
    closeCheck(fp, file);
  }

  return data;
}

static void unload(char** data) {
  assert(data != nullptr && *data != nullptr);

  free(*data);
  *data = nullptr;
}

static bool parseToken(const char* buffer, int row, int col) {
  if (buffer == nullptr || row < 0 || col < 0) {
    LOG_FATAL(game__log, "Error loading map: %s (unable to parse token)", FILE_MAZE);
    return false;
  }
  if (row >= MAZE_ROWS || col >= MAZE_COLS) {
    LOG_FATAL(game__log, "Error loading map: %s (incorrect number of rows or columns)", FILE_MAZE);
    return false;
  }

  errno = 0;
  char* end;
  int   tile = strtol(buffer, &end, 10);
  if (end == buffer || *end != '\0') {
    LOG_FATAL(game__log, "Error loading map: %s (invalid tile id)", FILE_MAZE);
    return false;
  }
  if (errno == ERANGE) {
    LOG_FATAL(game__log, "Error loading map: %s (strtol range error)", FILE_MAZE);
    return false;
  }

  g_maze[row][col] = tile;
  return true;
}

bool loadMaze(void) {
  char* maze = load(FILE_MAZE, "r");
  if (maze == nullptr) return false;

  char  buffer[BUFFER_SIZE];
  char* dst = buffer;

  // Process CSV
  char* src       = maze;
  int   row       = 0;
  int   col       = 0;
  int   tileCount = 0;
  while (*src != '\0') {
    if (*src == ',' || *src == '\n') {
      *dst = '\0';
      if (!parseToken(buffer, row, col)) {
        RETURN_FAIL();
      }

      dst = buffer;
      tileCount++;
      if (*src == '\n') {
        col = 0;
        row++;
      } else {
        col++;
      }
    } else {
      *dst++ = *src;
    }
    src++;
  }

  // Handle final token (ie file ends without newline)
  if (dst != buffer) {
    *dst = '\0';
    if (!parseToken(buffer, row, col)) {
      RETURN_FAIL();
    }
  }

  if (tileCount != MAZE_ROWS * MAZE_COLS) {
    LOG_FATAL(game__log, "Error loading map: %s (unexpected end of file)", FILE_MAZE);
    RETURN_FAIL();
  }

  unload(&maze);
  return true;
}

static void makeMazeAABB(void) {
  for (int row = 0; row < MAZE_ROWS; row++) {
    for (int col = 0; col < MAZE_COLS; col++) {
      g_mazeAABB[row][col] = (game__AABB) {
        .min = (Vector2) {       col * TILE_SIZE,       row * TILE_SIZE },
        .max = (Vector2) { (col + 1) * TILE_SIZE, (row + 1) * TILE_SIZE }
      };
    }
  }
}

// --- Maze functions ---

bool maze_init(void) {
  GAME_TRY(loadMaze());
  makeMazeAABB();
  return true;
}

game__AABB maze_getAABB(Vector2 pos) {
  int row = (int) (pos.y / TILE_SIZE);
  int col = (int) (pos.x / TILE_SIZE);
  assert(row >= 0 && row < MAZE_ROWS);
  assert(col >= 0 && col < MAZE_COLS);
  return g_mazeAABB[row][col];
}

bool maze_isWall(Vector2 pos) {
  int row = (int) (pos.y / TILE_SIZE);
  int col = (int) (pos.x / TILE_SIZE);
  assert(row >= 0 && row < MAZE_ROWS);
  assert(col >= 0 && col < MAZE_COLS);
  return g_maze[row][col];
}

void maze_tilesOverlay(void) {
  for (int row = 0; row < MAZE_ROWS; row++) {
    for (int col = 0; col < MAZE_COLS; col++) {
      if (maze_isWall(g_mazeAABB[row][col].min)) {
        aabb_drawOverlay(g_mazeAABB[row][col], OVERLAY_COLOUR_MAZE_WALL);
      }
    }
  }
}
