#include <stdio.h>
#include "common/config.h"
#include "common/logging.h"
#include "common/errors.h"
#include "common/utils.h"

// A dummy function to demonstrate error propagation
static syscore_error_t perform_check(int value) {
    if (value < 0) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }
    return SYSCORE_SUCCESS;
}

static syscore_error_t run_demo(void) {
    SYSCORE_LOG_INFO("Demonstrating error checking macro...");
    
    // This will succeed
    SYSCORE_RETURN_IF_ERROR(perform_check(42));
    
    // This will fail and log the error via SYSCORE_RETURN_IF_ERROR
    SYSCORE_RETURN_IF_ERROR(perform_check(-1));
    
    return SYSCORE_SUCCESS;
}

int main(void) {
    // Initialize logging at DEBUG level to see all logs
    syscore_log_init(SYSCORE_LOG_DEBUG);

    SYSCORE_LOG_INFO("SysCore initialized.");
    
    // Log at all levels
    SYSCORE_LOG_DEBUG("This is a debug log. Version: %s", syscore_get_version_string());
    SYSCORE_LOG_INFO("This is an info log.");
    SYSCORE_LOG_WARN("This is a warning log.");
    SYSCORE_LOG_ERROR("This is an error log.");

    // Detect Compiler and OS
#if defined(SYSCORE_OS_MACOS)
    SYSCORE_LOG_INFO("Detected OS: macOS");
#elif defined(SYSCORE_OS_LINUX)
    SYSCORE_LOG_INFO("Detected OS: Linux");
#elif defined(SYSCORE_OS_WINDOWS)
    SYSCORE_LOG_INFO("Detected OS: Windows");
#else
    SYSCORE_LOG_INFO("Detected OS: Unknown");
#endif

#if defined(SYSCORE_COMPILER_CLANG)
    SYSCORE_LOG_INFO("Detected Compiler: Clang");
#elif defined(SYSCORE_COMPILER_GCC)
    SYSCORE_LOG_INFO("Detected Compiler: GCC");
#elif defined(SYSCORE_COMPILER_MSVC)
    SYSCORE_LOG_INFO("Detected Compiler: MSVC");
#else
    SYSCORE_LOG_INFO("Detected Compiler: Unknown");
#endif

    // Run the error propagation demo
    syscore_error_t result = run_demo();
    if (result != SYSCORE_SUCCESS) {
        SYSCORE_LOG_WARN("Demo run completed with expected failure: %s", syscore_error_string(result));
    } else {
        SYSCORE_LOG_INFO("Demo run completed successfully.");
    }

    return 0;
}

