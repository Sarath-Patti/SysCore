#include "ipc/ipc.h"
#include "process/process.h"
#include "common/logging.h"
#include "common/errors.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

int main(void) {
    syscore_log_init(SYSCORE_LOG_DEBUG);
    SYSCORE_LOG_INFO("Starting Anonymous Pipe Demo");

    syscore_ipc_handle_t read_fd = SYSCORE_IPC_INVALID_HANDLE;
    syscore_ipc_handle_t write_fd = SYSCORE_IPC_INVALID_HANDLE;

    // Create the pipe
    syscore_error_t err = syscore_ipc_pipe_create(&read_fd, &write_fd);
    if (err != SYSCORE_SUCCESS) {
        SYSCORE_LOG_ERROR("Failed to create pipe: %s", syscore_error_string(err));
        return 1;
    }

    syscore_pid_t pid;
    err = syscore_process_fork(&pid);
    if (err != SYSCORE_SUCCESS) {
        SYSCORE_LOG_ERROR("Failed to fork: %s", syscore_error_string(err));
        syscore_ipc_close(read_fd);
        syscore_ipc_close(write_fd);
        return 1;
    }

    if (pid == 0) {
        // Child process: reads from pipe
        // Close unused write end
        syscore_ipc_close(write_fd);

        char buffer[128];
        size_t bytes_read = 0;
        SYSCORE_LOG_INFO("[Child] Waiting to read from pipe...");
        
        syscore_error_t read_err = syscore_ipc_read(read_fd, buffer, sizeof(buffer) - 1, &bytes_read);
        if (read_err != SYSCORE_SUCCESS) {
            SYSCORE_LOG_ERROR("[Child] Read failed: %s", syscore_error_string(read_err));
            syscore_ipc_close(read_fd);
            return 1;
        }

        buffer[bytes_read] = '\0';
        SYSCORE_LOG_INFO("[Child] Read %zu bytes: \"%s\"", bytes_read, buffer);

        syscore_ipc_close(read_fd);
        return 0;
    } else {
        // Parent process: writes to pipe
        // Close unused read end
        syscore_ipc_close(read_fd);

        const char* msg = "Hello from parent process via anonymous pipe!";
        size_t bytes_written = 0;
        SYSCORE_LOG_INFO("[Parent] Writing message to pipe...");

        syscore_error_t write_err = syscore_ipc_write(write_fd, msg, strlen(msg), &bytes_written);
        if (write_err != SYSCORE_SUCCESS) {
            SYSCORE_LOG_ERROR("[Parent] Write failed: %s", syscore_error_string(write_err));
            syscore_ipc_close(write_fd);
            return 1;
        }

        SYSCORE_LOG_INFO("[Parent] Wrote %zu bytes", bytes_written);
        syscore_ipc_close(write_fd);

        // Wait for child
        int status = 0;
        syscore_process_wait(pid, &status, 0);
        SYSCORE_LOG_INFO("[Parent] Child finished");
    }

    SYSCORE_LOG_INFO("Anonymous Pipe Demo completed successfully");
    return 0;
}
