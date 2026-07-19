#ifndef SYSCORE_COMMON_VERSION_H
#define SYSCORE_COMMON_VERSION_H

#define SYSCORE_VERSION_MAJOR 0
#define SYSCORE_VERSION_MINOR 9
#define SYSCORE_VERSION_PATCH 0
#define SYSCORE_VERSION_STRING "0.9.0"

/**
 * Returns the version string of the SysCore library.
 */
const char *syscore_get_version_string(void);

#endif // SYSCORE_COMMON_VERSION_H
