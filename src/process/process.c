#include "process/process.h"
#include "common/logging.h"
#include "common/errors.h"
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

static syscore_error_t map_errno(int err_num) {
    switch (err_num) {
        case 0:           return SYSCORE_SUCCESS;
        case EACCES:      return SYSCORE_ERROR_PERMISSION_DENIED;
        case EAGAIN:      return SYSCORE_ERROR_BUSY;
        case EFAULT:      return SYSCORE_ERROR_INVALID_ARGUMENT;
        case EINVAL:      return SYSCORE_ERROR_INVALID_ARGUMENT;
        case ENOENT:      return SYSCORE_ERROR_NOT_FOUND;
        case ENOMEM:      return SYSCORE_ERROR_OUT_OF_MEMORY;
        case EPERM:       return SYSCORE_ERROR_PERMISSION_DENIED;
        case ESRCH:       return SYSCORE_ERROR_NOT_FOUND;
        case ETIMEDOUT:   return SYSCORE_ERROR_TIMEOUT;
        default:          return SYSCORE_ERROR_GENERIC;
    }
}

syscore_error_t syscore_process_fork(syscore_pid_t* out_pid) {
    if (out_pid == NULL) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }
    
    SYSCORE_LOG_DEBUG("Spawning child process via fork()...");
    pid_t pid = fork();
    if (pid < 0) {
        int err = errno;
        SYSCORE_LOG_ERROR("fork() failed: %s (errno %d)", strerror(err), err);
        return map_errno(err);
    }
    
    *out_pid = pid;
    return SYSCORE_SUCCESS;
}

syscore_error_t syscore_process_exec(const char* path, char* const argv[]) {
    if (path == NULL || argv == NULL) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }
    
    SYSCORE_LOG_DEBUG("Executing program: %s", path);
    execvp(path, argv);
    
    // If execvp returns, it means an error occurred
    int err = errno;
    SYSCORE_LOG_ERROR("execvp() failed: %s (errno %d)", strerror(err), err);
    return map_errno(err);
}

syscore_error_t syscore_process_wait(syscore_pid_t pid, int* out_status, int options) {
    SYSCORE_LOG_DEBUG("Waiting for child process (PID: %d, options: %d)...", (int)pid, options);
    
    int status = 0;
    pid_t res = waitpid(pid, &status, options);
    if (res < 0) {
        int err = errno;
        SYSCORE_LOG_ERROR("waitpid() failed: %s (errno %d)", strerror(err), err);
        return map_errno(err);
    }
    
    if (out_status != NULL) {
        *out_status = status;
    }
    
    return SYSCORE_SUCCESS;
}

syscore_pid_t syscore_process_get_pid(void) {
    return getpid();
}

syscore_pid_t syscore_process_get_ppid(void) {
    return getppid();
}
