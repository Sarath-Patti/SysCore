#include "common/logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static syscore_log_level_t g_min_log_level = SYSCORE_LOG_INFO;

void syscore_log_init(syscore_log_level_t min_level) {
    g_min_log_level = min_level;
}

static const char* level_to_string(syscore_log_level_t level) {
    switch (level) {
        case SYSCORE_LOG_DEBUG: return "DEBUG";
        case SYSCORE_LOG_INFO:  return "INFO";
        case SYSCORE_LOG_WARN:  return "WARN";
        case SYSCORE_LOG_ERROR: return "ERROR";
        default:                return "UNKNOWN";
    }
}

#if SYSCORE_LOG_USE_COLOR
static const char* level_to_color(syscore_log_level_t level) {
    switch (level) {
        case SYSCORE_LOG_DEBUG: return "\033[36m"; // Cyan
        case SYSCORE_LOG_INFO:  return "\033[32m"; // Green
        case SYSCORE_LOG_WARN:  return "\033[33m"; // Yellow
        case SYSCORE_LOG_ERROR: return "\033[31m"; // Red
        default:                return "\033[0m";
    }
}
#endif

void syscore_log_write(syscore_log_level_t level, const char* file, int line, const char* format, ...) {
    if (level < g_min_log_level) {
        return;
    }

    // Get current local time
    time_t raw_time;
    time(&raw_time);
    struct tm* time_info = localtime(&raw_time);
    char time_str[20];
    if (time_info != NULL) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
    } else {
        time_str[0] = '\0';
    }

    // Determine target stream
    FILE* stream = (level >= SYSCORE_LOG_WARN) ? stderr : stdout;

    #if SYSCORE_LOG_USE_COLOR
    fprintf(stream, "%s %s[%-5s]\033[0m [%s:%d] ", time_str, level_to_color(level), level_to_string(level), file, line);
    #else
    fprintf(stream, "%s [%-5s] [%s:%d] ", time_str, level_to_string(level), file, line);
    #endif

    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);

    fprintf(stream, "\n");
    fflush(stream);
}
