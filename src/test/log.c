#include "../log/log.h"

#include <minunit.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Test default configuration
MU_TEST(test_defaultConfig) {
  const LogConfig* defaultConfig = log_getDefaultConfig();
  mu_check(defaultConfig != nullptr);
  mu_check(defaultConfig->minLevel == LOG_LEVEL_INFO);
  mu_check(defaultConfig->useColours == true);
  mu_check(defaultConfig->showTimestamp == true);
  mu_check(defaultConfig->showFileLine == true);
  mu_check(strcmp(defaultConfig->subsystem, "MAIN") == 0);
}

// Test log creation with nullptr config (uses defaults)
MU_TEST(test_createWithnullptrConfig) {
  Log* log = log_create(nullptr);
  mu_check(log != nullptr);

  const LogConfig* config = log_getConfig(log);
  mu_check(config != nullptr);
  mu_check(config->minLevel == LOG_LEVEL_INFO);
  mu_check(config->useColours == true);
  mu_check(config->showTimestamp == true);
  mu_check(config->showFileLine == true);
  mu_check(strcmp(config->subsystem, "MAIN") == 0);

  log_destroy(&log);
  mu_check(log == nullptr);
}

// Test log creation with custom config
MU_TEST(test_createWithCustomConfig) {
  LogConfig config = {
    .minLevel      = LOG_LEVEL_DEBUG,
    .useColours    = false,
    .showTimestamp = false,
    .showFileLine  = false,
    .subsystem     = "TEST"
  };

  Log* log = log_create(&config);
  mu_check(log != nullptr);

  const LogConfig* retrievedConfig = log_getConfig(log);
  mu_check(retrievedConfig != nullptr);
  mu_check(retrievedConfig->minLevel == LOG_LEVEL_DEBUG);
  mu_check(retrievedConfig->useColours == false);
  mu_check(retrievedConfig->showTimestamp == false);
  mu_check(retrievedConfig->showFileLine == false);
  mu_check(strcmp(retrievedConfig->subsystem, "TEST") == 0);

  log_destroy(&log);
  mu_check(log == nullptr);
}

// Test invalid config values
MU_TEST(test_invalidConfigs) {
  // Test invalid log level
  LogConfig invalidLevelConfig = {
    .minLevel      = LOG_LEVEL_COUNT,  // Invalid level
    .useColours    = true,
    .showTimestamp = true,
    .showFileLine  = true,
    .subsystem     = "TEST"
  };
  Log* log = log_create(&invalidLevelConfig);
  mu_check(log == nullptr);

  // Test empty subsystem name
  LogConfig emptySubsystemConfig = {
    .minLevel      = LOG_LEVEL_INFO,
    .useColours    = true,
    .showTimestamp = true,
    .showFileLine  = true,
    .subsystem     = ""  // Empty string
  };
  log = log_create(&emptySubsystemConfig);
  mu_check(log == nullptr);
}

// Test all log levels
MU_TEST(test_allLogLevels) {
  LogConfig config = {
    .minLevel      = LOG_LEVEL_TRACE,  // Set to lowest to test all levels
    .useColours    = true,
    .showTimestamp = true,
    .showFileLine  = true,
    .subsystem     = "TEST"
  };

  Log* log = log_create(&config);
  mu_check(log != nullptr);

  // Test all log levels
  LOG_TRACE(log, "This is a TRACE message");
  LOG_DEBUG(log, "This is a DEBUG message");
  LOG_INFO(log, "This is an INFO message");
  LOG_WARN(log, "This is a WARN message");
  LOG_ERROR(log, "This is an ERROR message");
  LOG_FATAL(log, "This is a FATAL message");

  log_destroy(&log);
  mu_check(log == nullptr);
}

// Test log level filtering
MU_TEST(test_logLevelFiltering) {
  // Only show WARN and above
  LogConfig config = {
    .minLevel      = LOG_LEVEL_WARN,
    .useColours    = true,
    .showTimestamp = true,
    .showFileLine  = true,
    .subsystem     = "TEST"
  };

  Log* log = log_create(&config);
  mu_check(log != nullptr);

  printf("\nThe following should only show WARN, ERROR, and FATAL:\n");
  LOG_TRACE(log, "This TRACE message should NOT be visible");
  LOG_DEBUG(log, "This DEBUG message should NOT be visible");
  LOG_INFO(log, "This INFO message should NOT be visible");
  LOG_WARN(log, "This WARN message should be visible");
  LOG_ERROR(log, "This ERROR message should be visible");
  LOG_FATAL(log, "This FATAL message should be visible");

  log_destroy(&log);
  mu_check(log == nullptr);
}

// Test different formatting options
MU_TEST(test_formattingOptions) {
  // Test with all formatting options disabled
  LogConfig minimalConfig = {
    .minLevel      = LOG_LEVEL_INFO,
    .useColours    = false,
    .showTimestamp = false,
    .showFileLine  = false,
    .subsystem     = nullptr  // No subsystem
  };

  Log* log = log_create(&minimalConfig);
  mu_check(log != nullptr);

  printf("\nMinimal formatting (no colors, timestamp, file/line):\n");
  LOG_INFO(log, "Minimal formatting message");

  log_destroy(&log);

  // Test with all formatting options enabled
  LogConfig fullConfig = {
    .minLevel      = LOG_LEVEL_INFO,
    .useColours    = true,
    .showTimestamp = true,
    .showFileLine  = true,
    .subsystem     = "FORMAT"
  };

  log = log_create(&fullConfig);
  mu_check(log != nullptr);

  printf("\nFull formatting:\n");
  LOG_INFO(log, "Full formatting message");

  log_destroy(&log);
  mu_check(log == nullptr);
}

// Test message formatting
MU_TEST(test_messageFormatting) {
  Log* log = log_create(nullptr);
  mu_check(log != nullptr);

  // Test string formatting
  LOG_INFO(log, "String: %s", "test string");

  // Test integer formatting
  LOG_INFO(log, "Integer: %d", 42);

  // Test float formatting
  LOG_INFO(log, "Float: %f", 3.14159);

  // Test multiple arguments
  LOG_INFO(log, "Multiple: %s %d %.2f", "test", 42, 3.14159);

  // Test empty format string
  LOG_INFO(log, "");

  log_destroy(&log);
  mu_check(log == nullptr);
}

// Test error handling in log_message
MU_TEST(test_logMessageErrors) {
  Log* log = log_create(nullptr);
  mu_check(log != nullptr);

  // These should print error messages to stderr but not crash
  printf("\nThe following should show error messages:\n");

  // nullptr format string
  log_message(log, LOG_LEVEL_INFO, __FILE__, __LINE__, true, nullptr);

  // nullptr file
  log_message(log, LOG_LEVEL_INFO, nullptr, __LINE__, true, "Test message");

  // Negative line number
  log_message(log, LOG_LEVEL_INFO, __FILE__, -1, true, "Test message");

  // Invalid log level
  log_message(log, LOG_LEVEL_COUNT, __FILE__, __LINE__, true, "Test message");

  // nullptr log
  log_message(nullptr, LOG_LEVEL_INFO, __FILE__, __LINE__, true, "Test message");

  log_destroy(&log);
  mu_check(log == nullptr);
}

// Test trailing newline parameter
MU_TEST(test_trailingNewline) {
  Log* log = log_create(nullptr);
  mu_check(log != nullptr);

  printf("\nTesting trailing newline parameter:\n");
  // With trailing newline (true)
  log_message(log, LOG_LEVEL_INFO, __FILE__, __LINE__, true, "With newline");

  // Without trailing newline (false)
  log_message(log, LOG_LEVEL_INFO, __FILE__, __LINE__, false, "Without newline");
  // This should appear on the same line as the previous message
  printf(" (this should be on the same line)\n");

  log_destroy(&log);
  mu_check(log == nullptr);
}

MU_TEST_SUITE(test_logSuite) {
  MU_RUN_TEST(test_defaultConfig);
  MU_RUN_TEST(test_createWithnullptrConfig);
  MU_RUN_TEST(test_createWithCustomConfig);
  MU_RUN_TEST(test_invalidConfigs);
  MU_RUN_TEST(test_allLogLevels);
  MU_RUN_TEST(test_logLevelFiltering);
  MU_RUN_TEST(test_formattingOptions);
  MU_RUN_TEST(test_messageFormatting);
  MU_RUN_TEST(test_logMessageErrors);
  MU_RUN_TEST(test_trailingNewline);
}

int main(void) {
  MU_RUN_SUITE(test_logSuite);
  MU_REPORT();

  return 0;
}
