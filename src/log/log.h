/**
 * @file log.h
 * @brief Logging sub-system
 *
 * Provides a flexible logging system with configurable log levels, colours,
 * timestamps, and file/line information.
 */

// clang-format Language: C
#pragma once

#include <stdarg.h>  // va_list

/**
 * @enum LogLevel
 * @brief Defines the severity levels for log messages
 */
typedef enum LogLevel {
  LOG_LEVEL_TRACE, /**< Ultra-fine-grained messages for tracing program execution flow */
  LOG_LEVEL_DEBUG, /**< Diagnostic debug messages useful during development */
  LOG_LEVEL_INFO,  /**< Coarse-grained informational messages about application progress */
  LOG_LEVEL_WARN,  /**< Warnings about potential issues that are non-fatal */
  LOG_LEVEL_ERROR, /**< Error messages indicating operations that have failed */
  LOG_LEVEL_FATAL, /**< Fatal errors after which the application must abort */
  LOG_LEVEL_COUNT  /**< Sentinel value representing the number of log levels */
} LogLevel;

/**
 * @struct LogConfig
 * @brief Configuration options for a logger instance
 */
typedef struct LogConfig {
  LogLevel minLevel;      /**< Minimum log level to display */
  bool     useColours;    /**< Whether to use coloured output */
  bool     showTimestamp; /**< Whether to show timestamps */
  bool     showFileLine;  /**< Whether to show file and line information */
  char*    subsystem;     /**< Subsystem name (e.g., "GFX", "AUDIO", "GAME") */
} LogConfig;

/**
 * @struct Log
 * @brief Opaque logger instance structure
 */
typedef struct Log Log;

/**
 * @brief Creates a new logger instance
 *
 * @param config Configuration options for the logger (optional)
 * @return Log* Pointer to the new logger instance, or NULL if creation failed
 *
 * @note If config is NULL, the default configuration will be used:
 *       - minLevel: LOG_LEVEL_INFO
 *       - useColours: true
 *       - showTimestamp: true
 *       - showFileLine: true
 *       - subsystem: "MAIN"
 */
Log* log_create(const LogConfig* config);

/**
 * @brief Destroys a logger instance and frees associated resources
 *
 * @param log Pointer to the logger pointer to destroy (will be set to NULL)
 */
void log_destroy(Log** log);

/**
 * @brief Gets the current configuration of a logger
 *
 * @param log The logger instance
 * @return const LogConfig* Pointer to the logger's configuration
 */
const LogConfig* log_getConfig(const Log* log);

/**
 * @brief Gets the default logger configuration
 *
 * @return const LogConfig* Pointer to the default configuration
 */
const LogConfig* log_getDefaultConfig(void);

/**
 * @brief Logs a formatted message with the specified level
 *
 * @param log Logger instance
 * @param level Severity level of the message
 * @param file Source file where the log occurred
 * @param line Line number where the log occurred
 * @param trailingNewline Whether to append a newline to the message
 * @param format Printf-style format string
 * @param ... Additional arguments for the format string
 */
void log_message(Log* log, LogLevel level, const char* file, int line, bool trailingNewline, const char* format, ...);

/**
 * @brief Logs a formatted message with the specified level using va_list
 *
 * @param log Logger instance
 * @param level Severity level of the message
 * @param file Source file where the log occurred
 * @param line Line number where the log occurred
 * @param trailingNewline Whether to append a newline to the message
 * @param format Printf-style format string
 * @param args Variable argument list
 */
void log_vmessage(
  Log*        log,
  LogLevel    level,
  const char* file,
  int         line,
  bool        trailingNewline,
  const char* format,
  va_list     args
);

// Convenience macros for different log levels
#define LOG_LOG(log, level, ...) log_message(log, level, __FILE__, __LINE__, true, __VA_ARGS__)
#define LOG_TRACE(log, ...)      LOG_LOG(log, LOG_LEVEL_TRACE, __VA_ARGS__)
#define LOG_DEBUG(log, ...)      LOG_LOG(log, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(log, ...)       LOG_LOG(log, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(log, ...)       LOG_LOG(log, LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERROR(log, ...)      LOG_LOG(log, LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(log, ...)      LOG_LOG(log, LOG_LEVEL_FATAL, __VA_ARGS__)
