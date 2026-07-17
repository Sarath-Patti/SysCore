#ifndef SYSCORE_COMMON_UTILS_H
#define SYSCORE_COMMON_UTILS_H

/* OS Detection */
#if defined(_WIN32) || defined(_WIN64)
    #define SYSCORE_OS_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
    #define SYSCORE_OS_MACOS 1
#elif defined(__linux__)
    #define SYSCORE_OS_LINUX 1
#else
    #define SYSCORE_OS_UNKNOWN 1
#endif

/* Compiler Detection */
#if defined(__clang__)
    #define SYSCORE_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define SYSCORE_COMPILER_GCC 1
#elif defined(_MSC_VER)
    #define SYSCORE_COMPILER_MSVC 1
#else
    #define SYSCORE_COMPILER_UNKNOWN 1
#endif

/* Useful Utility Macros */

/* Mark parameter or variable as unused to suppress compiler warnings */
#define SYSCORE_UNUSED(x) (void)(x)

/* Calculate number of elements in an array */
#define SYSCORE_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Min/Max utility macros */
#define SYSCORE_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SYSCORE_MAX(a, b) (((a) > (b)) ? (a) : (b))

/* Warn if return value is discarded */
#if defined(SYSCORE_COMPILER_GCC) || defined(SYSCORE_COMPILER_CLANG)
    #define SYSCORE_NODISCARD __attribute__((warn_unused_result))
#elif defined(SYSCORE_COMPILER_MSVC)
    #define SYSCORE_NODISCARD _Check_return_
#else
    #define SYSCORE_NODISCARD
#endif

#endif // SYSCORE_COMMON_UTILS_H
