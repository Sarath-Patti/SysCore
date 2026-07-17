#ifndef SYSCORE_PROCESS_H
#define SYSCORE_PROCESS_H

#include "common/errors.h"
#include <sys/types.h>

typedef pid_t syscore_pid_t;

/**
 * Spawns a new child process by duplicating the calling process (wrapper for fork).
 * On success, the PID of the child process is returned in the child_pid pointer.
 * The return value of the function itself is:
 * - SYSCORE_SUCCESS for the parent process, with child_pid set to the child's PID.
 * - SYSCORE_SUCCESS for the child process, with child_pid set to 0.
 * On error, a negative syscore_error_t is returned.
 *
 * @param out_pid Pointer to where the child's process ID is stored.
 */
syscore_error_t syscore_process_fork(syscore_pid_t* out_pid);

/**
 * Replaces the current process image with a new process image (wrapper for execvp).
 * This function only returns if an error occurs.
 * On error, a negative syscore_error_t is returned.
 *
 * @param path The path of the executable program.
 * @param argv An array of null-terminated strings representing the argument list.
 */
syscore_error_t syscore_process_exec(const char* path, char* const argv[]);

/**
 * Waits for a specific child process to change state (wrapper for waitpid).
 * If pid is -1, it waits for any child process.
 * On success, returns SYSCORE_SUCCESS and updates out_status with the exit status of the child.
 *
 * @param pid The PID of the child process to wait for, or -1 to wait for any.
 * @param out_status Pointer to an integer where the exit status will be written.
 * @param options Flags (e.g. WNOHANG) or 0.
 */
syscore_error_t syscore_process_wait(syscore_pid_t pid, int* out_status, int options);

/**
 * Retrieves the process ID of the current process.
 */
syscore_pid_t syscore_process_get_pid(void);

/**
 * Retrieves the parent process ID of the current process.
 */
syscore_pid_t syscore_process_get_ppid(void);

#endif // SYSCORE_PROCESS_H
