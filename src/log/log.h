/*
 * Logging sub-system
 *
 * Provides a flexible logging system with configurable log levels, colours,
 * timestamps, and file/line information.
 */

// clang-format Language: C
#pragma once

typedef enum LogLevel {
  LOG_LEVEL_TRACE,  // Ultra-fine-grained messages for tracing program execution flow
  LOG_LEVEL_DEBUG,  // Diagnostic debug messages useful during development
  LOG_LEVEL_INFO,   // Coarse-grained informational messages about application progress
  LOG_LEVEL_WARN,   // Warnings about potential issues that are non-fatal
  LOG_LEVEL_ERROR,  // Error messages indicating operations that have failed
  LOG_LEVEL_FATAL,  // Fatal errors after which the application must abort
  LOG_LEVEL_COUNT   // Sentinel value representing the number of log levels
} LogLevel;

typedef struct LogConfig {
  LogLevel minLevel;       // Minimum log level to display
  bool     useColours;     // Whether to use coloured output
  bool     showTimestamp;  // Whether to show timestamps
  bool     showFileLine;   // Whether to show file and line information
  char*    subsystem;      // Subsystem name (e.g., "GFX", "AUDIO", "GAME")
} LogConfig;

typedef struct Log Log;

// Returns nullptr if the log instance could not be created
// NOTE: The config is optional, if not provided the default config will be used:
//       - minLevel: LOG_LEVEL_INFO
//       - useColours: true
//       - showTimestamp: true
//       - showFileLine: true
//       - subsystem: "MAIN"
Log* log_create(const LogConfig* config);

void log_destroy(Log** log);

const LogConfig* log_getConfig(const Log* log);

const LogConfig* log_getDefaultConfig(void);

void log_message(Log* log, LogLevel level, const char* file, int line, bool trailingNewline, const char* format, ...);

// Convenience macros for different log levels
#define LOG_LOG(log, level, ...) log_message(log, level, __FILE__, __LINE__, true, __VA_ARGS__)
#define LOG_TRACE(log, ...)      LOG_LOG(log, LOG_LEVEL_TRACE, __VA_ARGS__)
#define LOG_DEBUG(log, ...)      LOG_LOG(log, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(log, ...)       LOG_LOG(log, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(log, ...)       LOG_LOG(log, LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERROR(log, ...)      LOG_LOG(log, LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(log, ...)      LOG_LOG(log, LOG_LEVEL_FATAL, __VA_ARGS__)
