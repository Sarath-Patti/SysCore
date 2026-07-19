#include "ipc/ipc.h"
#include "process/process.h"
#include "common/logging.h"
#include "common/errors.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/syscore_demo_fifo"

int main(void) {
    syscore_log_init(SYSCORE_LOG_DEBUG);
    SYSCORE_LOG_INFO("Starting Named FIFO Demo");

    // Create the FIFO
    syscore_error_t err = syscore_ipc_fifo_create(FIFO_PATH, 0666);
    if (err != SYSCORE_SUCCESS) {
        SYSCORE_LOG_ERROR("Failed to create FIFO: %s", syscore_error_string(err));
        return 1;
    }

    syscore_pid_t pid;
    err = syscore_process_fork(&pid);
    if (err != SYSCORE_SUCCESS) {
        SYSCORE_LOG_ERROR("Failed to fork: %s", syscore_error_string(err));
        return 1;
    }

    if (pid == 0) {
        // Child process: reads from FIFO
        syscore_ipc_handle_t fifo_read = SYSCORE_IPC_INVALID_HANDLE;
        
        SYSCORE_LOG_INFO("[Child] Opening FIFO for reading...");
        syscore_error_t open_err = syscore_ipc_fifo_open(FIFO_PATH, 0, &fifo_read);
        if (open_err != SYSCORE_SUCCESS) {
            SYSCORE_LOG_ERROR("[Child] Failed to open FIFO: %s", syscore_error_string(open_err));
            return 1;
        }

        char buffer[128];
        size_t bytes_read = 0;
        SYSCORE_LOG_INFO("[Child] Reading from FIFO...");
        
        syscore_error_t read_err = syscore_ipc_read(fifo_read, buffer, sizeof(buffer) - 1, &bytes_read);
        if (read_err != SYSCORE_SUCCESS) {
            SYSCORE_LOG_ERROR("[Child] Read failed: %s", syscore_error_string(read_err));
            syscore_ipc_close(fifo_read);
            return 1;
        }

        buffer[bytes_read] = '\0';
        SYSCORE_LOG_INFO("[Child] Read %zu bytes: \"%s\"", bytes_read, buffer);

        syscore_ipc_close(fifo_read);
        return 0;
    } else {
        // Parent process: writes to FIFO
        syscore_ipc_handle_t fifo_write = SYSCORE_IPC_INVALID_HANDLE;
        
        // Wait a brief moment to let child attempt to open first
        struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000000};
        nanosleep(&ts, NULL);

        SYSCORE_LOG_INFO("[Parent] Opening FIFO for writing...");
        syscore_error_t open_err = syscore_ipc_fifo_open(FIFO_PATH, 1, &fifo_write);
        if (open_err != SYSCORE_SUCCESS) {
            SYSCORE_LOG_ERROR("[Parent] Failed to open FIFO: %s", syscore_error_string(open_err));
            return 1;
        }

        const char* msg = "Hello from parent process via named FIFO!";
        size_t bytes_written = 0;
        SYSCORE_LOG_INFO("[Parent] Writing message to FIFO...");

        syscore_error_t write_err = syscore_ipc_write(fifo_write, msg, strlen(msg), &bytes_written);
        if (write_err != SYSCORE_SUCCESS) {
            SYSCORE_LOG_ERROR("[Parent] Write failed: %s", syscore_error_string(write_err));
            syscore_ipc_close(fifo_write);
            return 1;
        }

        SYSCORE_LOG_INFO("[Parent] Wrote %zu bytes", bytes_written);
        syscore_ipc_close(fifo_write);

        // Wait for child
        int status = 0;
        syscore_process_wait(pid, &status, 0);
        SYSCORE_LOG_INFO("[Parent] Child finished");
        
        // Unlink FIFO to clean up filesystem
        unlink(FIFO_PATH);
    }

    SYSCORE_LOG_INFO("Named FIFO Demo completed successfully");
    return 0;
}
