#include "ipc/ipc.h"
#include "common/logging.h"
#include "common/errors.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static syscore_error_t map_errno(int err_num) {
    switch (err_num) {
        case 0:           return SYSCORE_SUCCESS;
        case EACCES:      return SYSCORE_ERROR_PERMISSION_DENIED;
        case EEXIST:      return SYSCORE_SUCCESS; // Allow mkfifo EEXIST to be treated as success
        case EAGAIN:      return SYSCORE_ERROR_BUSY;
        case EFAULT:      return SYSCORE_ERROR_INVALID_ARGUMENT;
        case EINVAL:      return SYSCORE_ERROR_INVALID_ARGUMENT;
        case ENOENT:      return SYSCORE_ERROR_NOT_FOUND;
        case ENOMEM:      return SYSCORE_ERROR_OUT_OF_MEMORY;
        case EPERM:       return SYSCORE_ERROR_PERMISSION_DENIED;
        case EPIPE:       return SYSCORE_ERROR_IO;
        default:          return SYSCORE_ERROR_GENERIC;
    }
}

syscore_error_t syscore_ipc_pipe_create(syscore_ipc_handle_t* out_read_handle, syscore_ipc_handle_t* out_write_handle) {
    if (out_read_handle == NULL || out_write_handle == NULL) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }

    SYSCORE_LOG_DEBUG("Creating anonymous pipe...");
    int fds[2];
    if (pipe(fds) < 0) {
        int err = errno;
        SYSCORE_LOG_ERROR("pipe() failed: %s (errno %d)", strerror(err), err);
        return map_errno(err);
    }

    *out_read_handle = fds[0];
    *out_write_handle = fds[1];
    return SYSCORE_SUCCESS;
}

syscore_error_t syscore_ipc_fifo_create(const char* path, int mode) {
    if (path == NULL) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }

    SYSCORE_LOG_DEBUG("Creating named FIFO at path: %s (mode: 0%o)...", path, mode);
    if (mkfifo(path, (mode_t)mode) < 0) {
        int err = errno;
        if (err == EEXIST) {
            SYSCORE_LOG_DEBUG("Named FIFO already exists at path: %s", path);
            return SYSCORE_SUCCESS;
        }
        SYSCORE_LOG_ERROR("mkfifo() failed: %s (errno %d)", strerror(err), err);
        return map_errno(err);
    }

    return SYSCORE_SUCCESS;
}

syscore_error_t syscore_ipc_fifo_open(const char* path, int write_mode, syscore_ipc_handle_t* out_handle) {
    if (path == NULL || out_handle == NULL) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }

    int flags = write_mode ? O_WRONLY : O_RDONLY;
    SYSCORE_LOG_DEBUG("Opening named FIFO at path: %s (mode: %s)...", path, write_mode ? "write" : "read");

    int fd = open(path, flags);
    if (fd < 0) {
        int err = errno;
        SYSCORE_LOG_ERROR("open() FIFO failed: %s (errno %d)", strerror(err), err);
        return map_errno(err);
    }

    *out_handle = fd;
    return SYSCORE_SUCCESS;
}

syscore_error_t syscore_ipc_close(syscore_ipc_handle_t handle) {
    if (handle < 0) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }

    SYSCORE_LOG_DEBUG("Closing IPC handle: %d", handle);
    if (close(handle) < 0) {
        int err = errno;
        SYSCORE_LOG_ERROR("close() failed: %s (errno %d)", strerror(err), err);
        return map_errno(err);
    }

    return SYSCORE_SUCCESS;
}

syscore_error_t syscore_ipc_read(syscore_ipc_handle_t handle, void* buffer, size_t count, size_t* out_read_bytes) {
    if (handle < 0 || buffer == NULL) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }

    ssize_t res = read(handle, buffer, count);
    if (res < 0) {
        int err = errno;
        SYSCORE_LOG_ERROR("read() failed on handle %d: %s (errno %d)", handle, strerror(err), err);
        return map_errno(err);
    }

    if (out_read_bytes != NULL) {
        *out_read_bytes = (size_t)res;
    }

    return SYSCORE_SUCCESS;
}

syscore_error_t syscore_ipc_write(syscore_ipc_handle_t handle, const void* buffer, size_t count, size_t* out_written_bytes) {
    if (handle < 0 || buffer == NULL) {
        return SYSCORE_ERROR_INVALID_ARGUMENT;
    }

    ssize_t res = write(handle, buffer, count);
    if (res < 0) {
        int err = errno;
        SYSCORE_LOG_ERROR("write() failed on handle %d: %s (errno %d)", handle, strerror(err), err);
        return map_errno(err);
    }

    if (out_written_bytes != NULL) {
        *out_written_bytes = (size_t)res;
    }

    return SYSCORE_SUCCESS;
}
