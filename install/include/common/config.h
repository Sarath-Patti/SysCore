#ifndef SYSCORE_COMMON_CONFIG_H
#define SYSCORE_COMMON_CONFIG_H

#include "common/version.h"

/* Compile-time configuration options */

/* Enable/disable DEBUG level logs.
 * Can be overridden by compiler definitions (-DSYSCORE_LOG_ENABLE_DEBUG=0/1).
 */
#ifndef SYSCORE_LOG_ENABLE_DEBUG
#define SYSCORE_LOG_ENABLE_DEBUG 1
#endif

/* Enable/disable ANSI color output for logging. */
#ifndef SYSCORE_LOG_USE_COLOR
#define SYSCORE_LOG_USE_COLOR 1
#endif

#endif // SYSCORE_COMMON_CONFIG_H
