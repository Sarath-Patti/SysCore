#ifndef SYSCORE_IPC_H
#define SYSCORE_IPC_H

#include "common/errors.h"
#include <stddef.h>

/**
 * Handle type representing a POSIX file descriptor for IPC operations.
 */
typedef int syscore_ipc_handle_t;

#define SYSCORE_IPC_INVALID_HANDLE (-1)

/**
 * Creates an anonymous pipe (wrapper for pipe).
 * On success, returns SYSCORE_SUCCESS and populates:
 * - out_read_handle: Handle for reading from the pipe.
 * - out_write_handle: Handle for writing to the pipe.
 *
 * @param out_read_handle Pointer to store the read handle.
 * @param out_write_handle Pointer to store the write handle.
 */
syscore_error_t syscore_ipc_pipe_create(syscore_ipc_handle_t* out_read_handle, syscore_ipc_handle_t* out_write_handle);

/**
 * Creates a named FIFO special file at the specified path (wrapper for mkfifo).
 * If the FIFO already exists, returns SYSCORE_SUCCESS.
 *
 * @param path The filesystem path for the named FIFO.
 * @param mode Permission bits (e.g. 0666).
 */
syscore_error_t syscore_ipc_fifo_create(const char* path, int mode);

/**
 * Opens an existing named FIFO (wrapper for open).
 *
 * @param path The filesystem path for the named FIFO.
 * @param write_mode If non-zero, opens for writing; otherwise, opens for reading.
 * @param out_handle Pointer to store the opened handle.
 */
syscore_error_t syscore_ipc_fifo_open(const char* path, int write_mode, syscore_ipc_handle_t* out_handle);

/**
 * Closes an IPC handle (wrapper for close).
 *
 * @param handle The IPC handle to close.
 */
syscore_error_t syscore_ipc_close(syscore_ipc_handle_t handle);

/**
 * Reads data from an IPC handle (wrapper for read).
 *
 * @param handle The IPC handle to read from.
 * @param buffer Pointer to the destination buffer.
 * @param count Number of bytes to read.
 * @param out_read_bytes Pointer to store the number of bytes actually read.
 */
syscore_error_t syscore_ipc_read(syscore_ipc_handle_t handle, void* buffer, size_t count, size_t* out_read_bytes);

/**
 * Writes data to an IPC handle (wrapper for write).
 *
 * @param handle The IPC handle to write to.
 * @param buffer Pointer to the source data buffer.
 * @param count Number of bytes to write.
 * @param out_written_bytes Pointer to store the number of bytes actually written.
 */
syscore_error_t syscore_ipc_write(syscore_ipc_handle_t handle, const void* buffer, size_t count, size_t* out_written_bytes);

#endif // SYSCORE_IPC_H
