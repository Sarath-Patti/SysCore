#ifndef SYSCORE_COMMON_LOGGING_H
#define SYSCORE_COMMON_LOGGING_H

#include "common/config.h"

typedef enum {
    SYSCORE_LOG_DEBUG = 0,
    SYSCORE_LOG_INFO,
    SYSCORE_LOG_WARN,
    SYSCORE_LOG_ERROR
} syscore_log_level_t;

/**
 * Initializes the logging system with a minimum log level filter.
 */
void syscore_log_init(syscore_log_level_t min_level);

/**
 * Writes a log message to the appropriate stream (stdout/stderr) with formatting.
 * Typically invoked through the helper macros.
 */
void syscore_log_write(syscore_log_level_t level, const char* file, int line, const char* format, ...);

/* Public logging macros */
#define SYSCORE_LOG_INFO(fmt, ...)  syscore_log_write(SYSCORE_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define SYSCORE_LOG_WARN(fmt, ...)  syscore_log_write(SYSCORE_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define SYSCORE_LOG_ERROR(fmt, ...) syscore_log_write(SYSCORE_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#if SYSCORE_LOG_ENABLE_DEBUG
#define SYSCORE_LOG_DEBUG(fmt, ...) syscore_log_write(SYSCORE_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define SYSCORE_LOG_DEBUG(fmt, ...) do { (void)(fmt); } while (0)
#endif

#endif // SYSCORE_COMMON_LOGGING_H
