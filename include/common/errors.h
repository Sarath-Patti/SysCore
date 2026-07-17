#ifndef SYSCORE_COMMON_ERRORS_H
#define SYSCORE_COMMON_ERRORS_H

#include "common/logging.h"

typedef enum {
    SYSCORE_SUCCESS = 0,
    SYSCORE_ERROR_GENERIC = -1,
    SYSCORE_ERROR_INVALID_ARGUMENT = -2,
    SYSCORE_ERROR_OUT_OF_MEMORY = -3,
    SYSCORE_ERROR_NOT_FOUND = -4,
    SYSCORE_ERROR_PERMISSION_DENIED = -5,
    SYSCORE_ERROR_TIMEOUT = -6,
    SYSCORE_ERROR_NOT_SUPPORTED = -7,
    SYSCORE_ERROR_IO = -8,
    SYSCORE_ERROR_BUSY = -9
} syscore_error_t;

/**
 * Returns a string description of the error code.
 */
const char* syscore_error_string(syscore_error_t err);

/**
 * Macro for consistent error checking and propagation.
 * If the expression returns a non-success error code, log the error and return it.
 */
#define SYSCORE_RETURN_IF_ERROR(expr) \
    do { \
        syscore_error_t _err = (expr); \
        if (_err != SYSCORE_SUCCESS) { \
            SYSCORE_LOG_ERROR("Error occurred: %s (code %d)", syscore_error_string(_err), _err); \
            return _err; \
        } \
    } while (0)

#endif // SYSCORE_COMMON_ERRORS_H
