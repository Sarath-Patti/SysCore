#include "process/process.h"
#include "common/logging.h"
#include "common/errors.h"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    // Initialize logging at DEBUG level to see everything
    syscore_log_init(SYSCORE_LOG_DEBUG);
    SYSCORE_LOG_INFO("Starting Process Module Demo");

    syscore_pid_t my_pid = syscore_process_get_pid();
    syscore_pid_t parent_pid = syscore_process_get_ppid();
    SYSCORE_LOG_INFO("Current Process PID: %d, Parent PID: %d", (int)my_pid, (int)parent_pid);

    syscore_pid_t child_pid;
    syscore_error_t err = syscore_process_fork(&child_pid);
    if (err != SYSCORE_SUCCESS) {
        SYSCORE_LOG_ERROR("Failed to fork: %s", syscore_error_string(err));
        return 1;
    }

    if (child_pid == 0) {
        // Child Process
        syscore_pid_t inner_pid = syscore_process_get_pid();
        syscore_pid_t inner_parent = syscore_process_get_ppid();
        SYSCORE_LOG_INFO("[Child] Child process running. PID: %d, Parent PID: %d", (int)inner_pid, (int)inner_parent);

        // We will execute the 'echo' utility to print a message
        char* const args[] = { "echo", "Hello from child process exec!", NULL };
        SYSCORE_LOG_INFO("[Child] Executing 'echo' via execvp...");
        
        syscore_error_t exec_err = syscore_process_exec("echo", args);
        
        // If we reach here, exec failed
        SYSCORE_LOG_ERROR("[Child] exec failed: %s", syscore_error_string(exec_err));
        return 1;
    } else {
        // Parent Process
        SYSCORE_LOG_INFO("[Parent] Forked child process with PID: %d", (int)child_pid);
        SYSCORE_LOG_INFO("[Parent] Waiting for child process to complete...");

        int status = 0;
        syscore_error_t wait_err = syscore_process_wait(child_pid, &status, 0);
        if (wait_err != SYSCORE_SUCCESS) {
            SYSCORE_LOG_ERROR("[Parent] Wait failed: %s", syscore_error_string(wait_err));
            return 1;
        }

        if (WIFEXITED(status)) {
            SYSCORE_LOG_INFO("[Parent] Child completed with exit status: %d", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            SYSCORE_LOG_INFO("[Parent] Child terminated by signal: %d", WTERMSIG(status));
        } else {
            SYSCORE_LOG_INFO("[Parent] Child terminated abnormally.");
        }
    }

    SYSCORE_LOG_INFO("Process Module Demo completed successfully");
    return 0;
}
