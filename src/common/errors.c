#include "common/errors.h"

const char* syscore_error_string(syscore_error_t err) {
    switch (err) {
        case SYSCORE_SUCCESS:                  return "Success";
        case SYSCORE_ERROR_GENERIC:            return "Generic error";
        case SYSCORE_ERROR_INVALID_ARGUMENT:   return "Invalid argument";
        case SYSCORE_ERROR_OUT_OF_MEMORY:      return "Out of memory";
        case SYSCORE_ERROR_NOT_FOUND:          return "Resource not found";
        case SYSCORE_ERROR_PERMISSION_DENIED:  return "Permission denied";
        case SYSCORE_ERROR_TIMEOUT:            return "Operation timed out";
        case SYSCORE_ERROR_NOT_SUPPORTED:      return "Operation not supported";
        case SYSCORE_ERROR_IO:                 return "I/O error";
        case SYSCORE_ERROR_BUSY:               return "Resource busy";
        default:                               return "Unknown error";
    }
}
