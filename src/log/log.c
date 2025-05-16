/*
 * Implementation of the logging sub-system
 */

#include "log.h"

#include <assert.h>  // assert
#include <stdarg.h>  // va_list, va_start, va_end
#include <stdio.h>   // fprintf, stderr, stdout, perror
#include <stdlib.h>  // malloc, free, strdup
#include <string.h>  // strlen
#include <time.h>    // time, localtime, strftime

// --- Constants ---

// ANSI color codes for terminal output
constexpr char COLOUR_RESET[]   = "\033[0m";
constexpr char COLOUR_RED[]     = "\033[31m";
constexpr char COLOUR_GREEN[]   = "\033[32m";
constexpr char COLOUR_YELLOW[]  = "\033[33m";
constexpr char COLOUR_BLUE[]    = "\033[34m";
constexpr char COLOUR_MAGENTA[] = "\033[35m";
constexpr char COLOUR_CYAN[]    = "\033[36m";

constexpr int BUFFER_SIZE     = 128;
constexpr int TIMESTAMP_WIDTH = 8;
constexpr int SUBSYSTEM_WIDTH = 4;
constexpr int LEVEL_WIDTH     = 5;
constexpr int FILE_WIDTH      = 14;
constexpr int LINE_WIDTH      = 3;

static const char* LEVEL_NAMES[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

static const char* LEVEL_COLOURS[] = {
  COLOUR_CYAN, COLOUR_BLUE, COLOUR_GREEN, COLOUR_YELLOW, COLOUR_RED, COLOUR_MAGENTA
};

static const char TIME_ERROR[] = "[TIME ERROR]";

static const LogConfig defaultConfig = {
  .minLevel      = LOG_LEVEL_INFO,
  .useColours    = true,
  .showTimestamp = true,
  .showFileLine  = true,
  .subsystem     = "MAIN"
};

// --- Types ---

typedef struct TimestampCache {
  time_t lastUpdate;
  char   timestamp[BUFFER_SIZE];
} TimestampCache;

typedef struct Log {
  LogConfig      config;
  TimestampCache timestampCache;
  bool           ownsSubsystem;
} Log;

// --- Helpers ---

static inline bool fprintfCheck(FILE* file, const char* fmt, ...) {
  assert(file != nullptr && "File is null");
  assert(fmt != nullptr && "Format string is null");

  va_list ap;
  va_start(ap, fmt);
  int returnValue = vfprintf(file, fmt, ap);
  va_end(ap);

  if (returnValue < 0) {
    perror("fprintf() failed");
    return false;
  }

  return true;
}

static inline bool vfprintfCheck(FILE* file, const char* fmt, va_list args) {
  assert(file != nullptr && "File is null");
  assert(fmt != nullptr && "Format string is null");

  int returnValue = vfprintf(file, fmt, args);
  if (returnValue < 0) {
    perror("vfprintf() failed");
    return false;
  }

  return true;
}

static const char* getCachedTimestamp(Log* log) {
  assert(log != nullptr && "Log is null");

  time_t now;
  if (time(&now) == (time_t) -1) {
    perror("time() failed");
    return TIME_ERROR;
  }

  // Update cache if more than a second has passed
  if (now != log->timestampCache.lastUpdate) {
    struct tm* timeInfo = localtime(&now);
    if (timeInfo == nullptr) {
      perror("localtime() failed");
      return TIME_ERROR;
    }

    size_t timestampCount = strftime(
      log->timestampCache.timestamp,
      sizeof(log->timestampCache.timestamp),
      "%H:%M:%S",
      timeInfo
    );
    if (timestampCount == 0) {
      perror("strftime() failed");
      return TIME_ERROR;
    }

    log->timestampCache.lastUpdate = now;
  }

  return log->timestampCache.timestamp;
}

// --- Public API ---

Log* log_create(const LogConfig* newConfig) {
  Log* log = (Log*) malloc(sizeof(Log));
  if (log == nullptr) {
    perror("malloc() failed");
    return nullptr;
  }

  if (newConfig == nullptr) {
    log->config        = defaultConfig;
    log->ownsSubsystem = false;
  } else {
    // Validate log level
    if (newConfig->minLevel < 0 || newConfig->minLevel >= LOG_LEVEL_COUNT) {
      fprintfCheck(stderr, "Invalid log level: %d.\n", newConfig->minLevel);
      return nullptr;
    }

    // Validate subsystem name
    if (newConfig->subsystem != nullptr && strlen(newConfig->subsystem) == 0) {
      fprintfCheck(stderr, "Invalid subsystem name: empty string.\n");
      return nullptr;
    }

    // All validations passed, safe to copy
    log->config           = *newConfig;
    log->config.subsystem = strdup(newConfig->subsystem);
    if (log->config.subsystem == nullptr) {
      perror("strdup() failed");
      free(log);
      return nullptr;
    }
    log->ownsSubsystem = true;
  }

  return log;
}

void log_destroy(Log** log) {
  if (*log != nullptr) {
    if ((*log)->config.subsystem != nullptr && (*log)->ownsSubsystem) {
      free((*log)->config.subsystem);
    }
    free(*log);
    *log = nullptr;
  }
}

const LogConfig* log_getConfig(const Log* log) {
  if (log == nullptr) {
    return nullptr;
  }

  return &log->config;
}

const LogConfig* log_getDefaultConfig(void) {
  return &defaultConfig;
}

void log_message(Log* log, LogLevel level, const char* file, int line, bool trailingNewline, const char* format, ...) {
  // Validate parameters
  if (log == nullptr) {
    fprintfCheck(stderr, "Invalid log message: log is null\n");
    return;
  }
  if (format == nullptr) {
    fprintfCheck(stderr, "Invalid log message: format string is null\n");
    return;
  }
  if (file == nullptr) {
    fprintfCheck(stderr, "Invalid log message: file name is null\n");
    return;
  }
  if (line < 0) {
    fprintfCheck(stderr, "Invalid log message: line number is negative (%d)\n", line);
    return;
  }
  if (level < 0 || level >= LOG_LEVEL_COUNT) {
    fprintfCheck(stderr, "Invalid log message: log level out of range (%d)\n", level);
    return;
  }

  if (level < log->config.minLevel) {
    return;
  }

  FILE* stream = level >= LOG_LEVEL_ERROR ? stderr : stdout;

  if (log->config.showTimestamp) {
    if (!fprintfCheck(stream, "[%-*s] ", TIMESTAMP_WIDTH, getCachedTimestamp(log))) {
      return;
    }
  }

  if (log->config.subsystem) {
    if (!fprintfCheck(stream, "[%-*s] ", SUBSYSTEM_WIDTH, log->config.subsystem)) {
      return;
    }
  }

  if (log->config.useColours) {
    if (!fprintfCheck(stream, "%s[%-*s]%s ", LEVEL_COLOURS[level], LEVEL_WIDTH, LEVEL_NAMES[level], COLOUR_RESET)) {
      return;
    }
  } else {
    if (!fprintfCheck(stream, "[%-*s] ", LEVEL_WIDTH, LEVEL_NAMES[level])) {
      return;
    }
  }

  if (log->config.showFileLine) {
    if (!fprintfCheck(stream, "(%-*s:%-*d) ", FILE_WIDTH, file, LINE_WIDTH, line)) {
      return;
    }
  }

  va_list args;
  va_start(args, format);
  if (!vfprintfCheck(stream, format, args)) {
    va_end(args);
    return;
  }
  va_end(args);
  if (trailingNewline) {
    fprintfCheck(stream, "\n");
  }
}
