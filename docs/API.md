# API Overview

The SysCore API provides a modular C library that wraps platform-specific POSIX services. By mapping platform-specific behaviors to clean interfaces, it enables developers to build concurrent systems applications on POSIX platforms without dealing directly with raw system call differences.

### Design Philosophy
- **Consistent Naming**: All library interfaces follow the format:
  ```c
  syscore_<module>_<action>()
  ```
- **Explicit Error Returns**: Function executions return a `syscore_error_t` status. Data payload or descriptor outputs are returned via final parameter pointers.
- **Resource Ownership**: Subsystems use distinct lifecycle API pairs (e.g. init/destroy or create/unlink), giving caller applications clear management boundaries.

### Thread Safety Expectations
- Primitives such as Mutexes (`syscore_mutex_t`), Condition Variables (`syscore_cond_t`), and Semaphores (`syscore_sem_t`) are designed to coordinate thread synchronization.
- Unless otherwise documented, operations on shared resources must be synchronized by the application.

# Common Conventions

All SysCore APIs leverage unified signatures and resource lifecycle conventions:

| Convention | Description |
|---|---|
| **Return Values** | Functions return `syscore_error_t` (`SYSCORE_SUCCESS` on success, negative enum values on failure). |
| **Error Codes** | Specific enum errors mapped from OS-level `errno` identifiers (e.g., `SYSCORE_ERROR_IO`). |
| **Output Parameters** | Variables returning resources are pointers placed at the end of the argument list. |
| **Handle Ownership** | The application is responsible for closing open descriptors or freeing mappings. |
| **Resource Lifecycle** | Symmetrical allocation and deallocation functions (e.g. `open` and `close`). |
| **Naming Conventions** | Subsystems use lower_snake_case prefixes: `syscore_shm_`, `syscore_sem_`, etc. |
| **Header Organization** | Public headers are mapped to module namespaces: `include/memory/mmap.h`, etc. |

# Module Reference

---

## Common

This module manages version checking, logging, and error utility helpers.

| Function | Description |
|---|---|
| `syscore_get_version_string` | Retrieves the version string of the compiled SysCore library. |
| `syscore_error_string` | Translates an error code into a human-readable text string. |
| `syscore_log_init` | Initializes the structured logging filter level. |

### syscore_get_version_string

```c
const char *syscore_get_version_string(void);
```
- **Purpose**: Retrieves the version string of the compiled SysCore library.
- **Parameters**: None.
- **Return Value**: Pointer to a static, null-terminated version string (e.g., `"1.0.0"`).
- **Possible Errors**: None.
- **Notes**: The returned pointer points to static read-only memory and must not be freed.
- **Example**:
  ```c
  printf("SysCore version: %s\n", syscore_get_version_string());
  ```

### syscore_error_string

```c
const char *syscore_error_string(syscore_error_t err);
```
- **Purpose**: Translates an error code into a human-readable text string.
- **Parameters**:
  - `err`: The `syscore_error_t` code to translate.
- **Return Value**: Pointer to a static description string.
- **Possible Errors**: None.
- **Example**:
  ```c
  syscore_error_t err = syscore_process_fork(NULL);
  printf("Error: %s\n", syscore_error_string(err));
  ```

### syscore_log_init

```c
void syscore_log_init(syscore_log_level_t min_level);
```
- **Purpose**: Initializes the structured logging filter level.
- **Parameters**:
  - `min_level`: Minimum logging level to output (`SYSCORE_LOG_DEBUG`, `SYSCORE_LOG_INFO`, `SYSCORE_LOG_WARN`, `SYSCORE_LOG_ERROR`).
- **Return Value**: None.

---

## Process

Abstractions for system processes orchestration.

| Function | Description |
|---|---|
| `syscore_process_fork` | Spawns a new child process by duplicating the calling process. |
| `syscore_process_exec` | Replaces the calling process image with a new executable program image. |
| `syscore_process_wait` | Waits for a specific child process to change state. |
| `syscore_process_get_pid` | Retrieves the calling process's identifier (PID). |
| `syscore_process_get_ppid` | Retrieves the parent process's identifier (PPID). |

### syscore_process_fork

```c
syscore_error_t syscore_process_fork(syscore_pid_t* out_pid);
```
- **Purpose**: Spawns a new child process by duplicating the calling process.
- **Parameters**:
  - `out_pid`: Output pointer to store the child's process ID. On success, set to child's PID in the parent process, and `0` in the child process.
- **Return Value**: `SYSCORE_SUCCESS` on success, or a negative error code on failure.
- **Possible Errors**: `SYSCORE_ERROR_GENERIC` (system fork limits hit).
- **Example**:
  ```c
  syscore_pid_t pid;
  if (syscore_process_fork(&pid) == SYSCORE_SUCCESS) {
      if (pid == 0) {
          // Child
      }
  }
  ```

### syscore_process_exec

```c
syscore_error_t syscore_process_exec(const char* path, char* const argv[]);
```
- **Purpose**: Replaces the calling process image with a new executable program image.
- **Parameters**:
  - `path`: File path of the executable program to run.
  - `argv`: Null-terminated array of argument string pointers.
- **Return Value**: Only returns if an error occurs.
- **Possible Errors**: `SYSCORE_ERROR_NOT_FOUND` (executable missing), `SYSCORE_ERROR_PERMISSION_DENIED` (no execution rights).

### syscore_process_wait

```c
syscore_error_t syscore_process_wait(syscore_pid_t pid, int* out_status, int options);
```
- **Purpose**: Waits for a specific child process (or any child process if pid is -1) to terminate.
- **Parameters**:
  - `pid`: Process ID to wait for, or `-1` for any child.
  - `out_status`: Output pointer to receive the termination status bits.
  - `options`: Execution flags (e.g. `WNOHANG` or `0`).
- **Return Value**: `SYSCORE_SUCCESS` on success, or a negative error code on failure.

### syscore_process_get_pid

```c
syscore_pid_t syscore_process_get_pid(void);
```
- **Purpose**: Retrieves the calling process's unique identifier (PID).

### syscore_process_get_ppid

```c
syscore_pid_t syscore_process_get_ppid(void);
```
- **Purpose**: Retrieves the parent process's unique identifier (PPID).

---

## IPC

Abstraction layer for unidirectional pipes and local FIFO files.

| Function | Description |
|---|---|
| `syscore_ipc_pipe_create` | Creates a unidirectional data pipe. |
| `syscore_ipc_fifo_create` | Creates a named FIFO special file. |
| `syscore_ipc_fifo_open` | Opens an existing named FIFO. |
| `syscore_ipc_close` | Closes an active IPC file descriptor. |
| `syscore_ipc_read` | Reads data from an IPC handle descriptor. |
| `syscore_ipc_write` | Writes data to an IPC handle descriptor. |

### syscore_ipc_pipe_create

```c
syscore_error_t syscore_ipc_pipe_create(syscore_ipc_handle_t* out_read_handle, syscore_ipc_handle_t* out_write_handle);
```
- **Purpose**: Creates a unidirectional data pipe.
- **Parameters**:
  - `out_read_handle`: Pointer to store the read handle descriptor.
  - `out_write_handle`: Pointer to store the write handle descriptor.
- **Return Value**: `SYSCORE_SUCCESS` on success, or a negative error code.

### syscore_ipc_fifo_create

```c
syscore_error_t syscore_ipc_fifo_create(const char* path, int mode);
```
- **Purpose**: Creates a named FIFO special file at the specified path.
- **Parameters**:
  - `path`: The filesystem path for the named FIFO.
  - `mode`: Permission bits (e.g. `0666`).
- **Return Value**: `SYSCORE_SUCCESS` on success, or a negative error code.

### syscore_ipc_fifo_open

```c
syscore_error_t syscore_ipc_fifo_open(const char* path, int write_mode, syscore_ipc_handle_t* out_handle);
```
- **Purpose**: Opens an existing named FIFO.
- **Parameters**:
  - `path`: The filesystem path for the named FIFO.
  - `write_mode`: If non-zero, opens for writing; otherwise, opens for reading.
  - `out_handle`: Pointer to store the opened handle.
- **Return Value**: `SYSCORE_SUCCESS` on success, or a negative error code.

### syscore_ipc_close

```c
syscore_error_t syscore_ipc_close(syscore_ipc_handle_t handle);
```
- **Purpose**: Closes an active IPC file descriptor.

### syscore_ipc_read

```c
syscore_error_t syscore_ipc_read(syscore_ipc_handle_t handle, void* buffer, size_t count, size_t* out_read_bytes);
```
- **Purpose**: Reads data from an IPC handle descriptor.
- **Parameters**:
  - `handle`: Open read handle.
  - `buffer`: Target buffer to write bytes.
  - `count`: Maximum bytes count to read.
  - `out_read_bytes`: Pointer to receive the actual number of read bytes.

### syscore_ipc_write

```c
syscore_error_t syscore_ipc_write(syscore_ipc_handle_t handle, const void* buffer, size_t count, size_t* out_written_bytes);
```
- **Purpose**: Writes data to an IPC handle descriptor.
- **Parameters**:
  - `handle`: Open write handle.
  - `buffer`: Pointer to the source data buffer.
  - `count`: Number of bytes to write.
  - `out_written_bytes`: Pointer to store the number of bytes actually written.

---

## Threading

Execution thread spawning and thread parameters.

| Function | Description |
|---|---|
| `syscore_thread_attr_init` | Initializes thread attributes to default values. |
| `syscore_thread_create` | Spawns a new thread executing the start routine. |
| `syscore_thread_join` | Waits for the specified thread to terminate. |
| `syscore_thread_self` | Retrieves the handle of the calling thread. |
| `syscore_thread_sleep_ms` | Suspends execution of the calling thread for a duration in milliseconds. |

### syscore_thread_attr_init

```c
syscore_error_t syscore_thread_attr_init(syscore_thread_attr_t *attr);
```
- **Purpose**: Initializes thread attributes to default values.
- **Parameters**:
  - `attr`: Pointer to attributes structure to initialize.
- **Return Value**: `SYSCORE_SUCCESS` on success, or a negative error code.

### syscore_thread_create

```c
syscore_error_t syscore_thread_create(syscore_thread_t *out_thread, const syscore_thread_attr_t *attr, syscore_thread_func_t func, void *arg);
```
- **Purpose**: Spawns a new concurrent execution thread.
- **Parameters**:
  - `out_thread`: Pointer to store the spawned thread handle.
  - `attr`: Optional thread attributes (stack size, detach state). Can be `NULL` for default behavior.
  - `func`: The thread function entry point.
  - `arg`: Context argument pointer passed to the thread function.

### syscore_thread_join

```c
syscore_error_t syscore_thread_join(syscore_thread_t thread, void **out_retval);
```
- **Purpose**: Waits for the specified thread to terminate.
- **Parameters**:
  - `thread`: The thread handle to wait for.
  - `out_retval`: Pointer to store the return value of the thread function (can be `NULL`).
- **Return Value**: `SYSCORE_SUCCESS` on success, or a negative error code.

### syscore_thread_self

```c
syscore_thread_t syscore_thread_self(void);
```
- **Purpose**: Retrieves the handle of the calling thread.

### syscore_thread_sleep_ms

```c
void syscore_thread_sleep_ms(unsigned int ms);
```
- **Purpose**: Delays calling thread execution for the specified milliseconds.

---

## Synchronization

Standard abstractions wrapping mutexes, condition variables, and read-write locks.

| Function | Description |
|---|---|
| `syscore_mutex_init` | Initializes a mutual exclusion lock structure. |
| `syscore_mutex_lock` | Locks/acquires the specified mutex. |
| `syscore_mutex_unlock` | Unlocks/releases the specified mutex. |
| `syscore_mutex_destroy` | Destroys the specified mutex. |
| `syscore_cond_init` | Initializes a condition variable. |
| `syscore_cond_wait` | Blocks calling thread on a condition variable. |
| `syscore_cond_signal` | Signals/wakes one thread waiting on a condition variable. |
| `syscore_cond_broadcast` | Signals/wakes all threads waiting on a condition variable. |
| `syscore_cond_destroy` | Destroys a condition variable. |
| `syscore_rwlock_init` | Initializes a read-write lock. |
| `syscore_rwlock_rdlock` | Acquires a read lock on a read-write lock. |
| `syscore_rwlock_wrlock` | Acquires a write lock on a read-write lock. |
| `syscore_rwlock_unlock` | Releases a read-write lock. |
| `syscore_rwlock_destroy` | Destroys a read-write lock. |

### syscore_mutex_init

```c
syscore_error_t syscore_mutex_init(syscore_mutex_t *mutex);
```
- **Purpose**: Initializes a mutual exclusion lock structure.

### syscore_mutex_lock

```c
syscore_error_t syscore_mutex_lock(syscore_mutex_t *mutex);
```
- **Purpose**: Locks/acquires the specified mutex.

### syscore_mutex_unlock

```c
syscore_error_t syscore_mutex_unlock(syscore_mutex_t *mutex);
```
- **Purpose**: Unlocks/releases the specified mutex.

### syscore_mutex_destroy

```c
syscore_error_t syscore_mutex_destroy(syscore_mutex_t *mutex);
```
- **Purpose**: Destroys the specified mutex.

### syscore_cond_init

```c
syscore_error_t syscore_cond_init(syscore_cond_t *cond);
```
- **Purpose**: Initializes a condition variable.

### syscore_cond_wait

```c
syscore_error_t syscore_cond_wait(syscore_cond_t *cond, syscore_mutex_t *mutex);
```
- **Purpose**: Blocks execution of the calling thread, releasing the mutex until signaled by another thread.

### syscore_cond_signal

```c
syscore_error_t syscore_cond_signal(syscore_cond_t *cond);
```
- **Purpose**: Signals/wakes one thread waiting on a condition variable.

### syscore_cond_broadcast

```c
syscore_error_t syscore_cond_broadcast(syscore_cond_t *cond);
```
- **Purpose**: Signals/wakes all threads waiting on a condition variable.

### syscore_cond_destroy

```c
syscore_error_t syscore_cond_destroy(syscore_cond_t *cond);
```
- **Purpose**: Destroys a condition variable.

### syscore_rwlock_init

```c
syscore_error_t syscore_rwlock_init(syscore_rwlock_t *rwlock);
```
- **Purpose**: Initializes a read-write lock.

### syscore_rwlock_rdlock

```c
syscore_error_t syscore_rwlock_rdlock(syscore_rwlock_t *rwlock);
```
- **Purpose**: Acquires a read lock on a read-write lock.

### syscore_rwlock_wrlock

```c
syscore_error_t syscore_rwlock_wrlock(syscore_rwlock_t *rwlock);
```
- **Purpose**: Acquires a write lock on a read-write lock.

### syscore_rwlock_unlock

```c
syscore_error_t syscore_rwlock_unlock(syscore_rwlock_t *rwlock);
```
- **Purpose**: Releases a read-write lock.

### syscore_rwlock_destroy

```c
syscore_error_t syscore_rwlock_destroy(syscore_rwlock_t *rwlock);
```
- **Purpose**: Destroys a read-write lock.

---

## Shared Memory

Allocates and manages shared memory object handles and mappings shared across process bounds.

| Function | Description |
|---|---|
| `syscore_shm_create` | Creates a new shared memory object. |
| `syscore_shm_open` | Opens an existing shared memory object. |
| `syscore_shm_map` | Maps the shared memory object into the calling process's virtual address space. |
| `syscore_shm_unmap` | Unmaps the shared memory from the process's virtual address space. |
| `syscore_shm_close` | Closes the shared memory descriptor. |
| `syscore_shm_destroy` | Destroys/unlinks a shared memory object name. |

### syscore_shm_create

```c
syscore_error_t syscore_shm_create(const char *name, size_t size, int mode, syscore_shm_handle_t *out_handle);
```
- **Purpose**: Generates and opens a new shared memory object block.
- **Parameters**:
  - `name`: Shared object identifier path (must begin with a `/`).
  - `size`: Allocated block memory size in bytes.
  - `mode`: Permission bits (e.g. `0666`).
  - `out_handle`: Output pointer to receive the file descriptor handle.

### syscore_shm_open

```c
syscore_error_t syscore_shm_open(const char *name, int write_mode, syscore_shm_handle_t *out_handle);
```
- **Purpose**: Opens an existing shared memory object.
- **Parameters**:
  - `name`: Shared memory object path.
  - `write_mode`: If non-zero, open for reading and writing; otherwise, open for reading only.
  - `out_handle`: Output pointer to receive the handle.

### syscore_shm_map

```c
syscore_error_t syscore_shm_map(syscore_shm_handle_t handle, size_t size, int write_mode, void **out_addr);
```
- **Purpose**: Maps the shared memory object into the calling process's virtual address space.
- **Parameters**:
  - `handle`: Shared memory handle.
  - `size`: Size of the region to map.
  - `write_mode`: If non-zero, map with read-write permissions; otherwise, map read-only.
  - `out_addr`: Output pointer to receive mapped virtual memory address.

### syscore_shm_unmap

```c
syscore_error_t syscore_shm_unmap(void *addr, size_t size);
```
- **Purpose**: Unmaps the shared memory from the process's virtual address space.

### syscore_shm_close

```c
syscore_error_t syscore_shm_close(syscore_shm_handle_t handle);
```
- **Purpose**: Closes the shared memory descriptor.

### syscore_shm_destroy

```c
syscore_error_t syscore_shm_destroy(const char *name);
```
- **Purpose**: Destroys/unlinks a shared memory object name.

---

## Semaphores

value-based barriers supporting unnamed and named POSIX configurations.

| Function | Description |
|---|---|
| `syscore_sem_init` | Initializes an unnamed/anonymous semaphore. |
| `syscore_sem_destroy` | Destroys an unnamed semaphore. |
| `syscore_sem_open` | Creates or opens a named semaphore. |
| `syscore_sem_close` | Closes an opened named semaphore. |
| `syscore_sem_unlink` | Unlinks a named semaphore. |
| `syscore_sem_wait` | Decrements (locks) the semaphore. |
| `syscore_sem_trywait` | Non-blocking decrement of the semaphore. |
| `syscore_sem_post` | Increments (unlocks) the semaphore. |

### syscore_sem_init

```c
syscore_error_t syscore_sem_init(syscore_sem_t *sem, int shared, unsigned int value);
```
- **Purpose**: Initializes an unnamed/anonymous memory-backed semaphore.
- **Notes**: On macOS Darwin (where unnamed semaphores are deprecated), this function emulates unnamed semaphores under the hood using a unique named semaphore path.

### syscore_sem_destroy

```c
syscore_error_t syscore_sem_destroy(syscore_sem_t *sem);
```
- **Purpose**: Destroys an unnamed semaphore.

### syscore_sem_open

```c
syscore_error_t syscore_sem_open(const char *name, int mode, unsigned int value, syscore_sem_t *sem);
```
- **Purpose**: Creates or opens a named semaphore.

### syscore_sem_close

```c
syscore_error_t syscore_sem_close(syscore_sem_t *sem);
```
- **Purpose**: Closes an opened named semaphore.

### syscore_sem_unlink

```c
syscore_error_t syscore_sem_unlink(const char *name);
```
- **Purpose**: Unlinks a named semaphore.

### syscore_sem_wait

```c
syscore_error_t syscore_sem_wait(syscore_sem_t *sem);
```
- **Purpose**: Decrements (locks) the semaphore.

### syscore_sem_trywait

```c
syscore_error_t syscore_sem_trywait(syscore_sem_t *sem);
```
- **Purpose**: Non-blocking decrement of the semaphore.

### syscore_sem_post

```c
syscore_error_t syscore_sem_post(syscore_sem_t *sem);
```
- **Purpose**: Increments (unlocks) the semaphore.

---

## Message Queues

Abstractions for priority messaging queues.

| Function | Description |
|---|---|
| `syscore_mq_open` | Opens or creates a message queue. |
| `syscore_mq_close` | Closes the message queue descriptor. |
| `syscore_mq_unlink` | Destroys/unlinks the message queue name. |
| `syscore_mq_send` | Sends a message to the queue. |
| `syscore_mq_receive` | Receives a message from the queue. |
| `syscore_mq_get_attr` | Retrieves the current attributes of the queue. |
| `syscore_mq_set_attr` | Sets new attributes on the queue (only flags can be modified). |

### syscore_mq_open

```c
syscore_error_t syscore_mq_open(const char *name, int write_mode, int non_block, int mode, const syscore_mq_attr_t *attr, syscore_mq_handle_t *out_handle);
```
- **Purpose**: Creates or opens a message queue.
- **Notes**: macOS does not natively support message queues. On macOS, this initializes an internal emulation layer combining shared memory objects and named semaphores using a hashed (`djb2_hash`) name.

### syscore_mq_close

```c
syscore_error_t syscore_mq_close(syscore_mq_handle_t handle);
```
- **Purpose**: Closes the message queue descriptor.

### syscore_mq_unlink

```c
syscore_error_t syscore_mq_unlink(const char *name);
```
- **Purpose**: Destroys/unlinks the message queue name.

### syscore_mq_send

```c
syscore_error_t syscore_mq_send(syscore_mq_handle_t handle, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
```
- **Purpose**: Sends a message to the queue.

### syscore_mq_receive

```c
syscore_error_t syscore_mq_receive(syscore_mq_handle_t handle, char *msg_ptr, size_t msg_len, unsigned int *out_msg_prio, size_t *out_read_bytes);
```
- **Purpose**: Receives a message from the queue.

### syscore_mq_get_attr

```c
syscore_error_t syscore_mq_get_attr(syscore_mq_handle_t handle, syscore_mq_attr_t *attr);
```
- **Purpose**: Retrieves the current attributes of the queue.

### syscore_mq_set_attr

```c
syscore_error_t syscore_mq_set_attr(syscore_mq_handle_t handle, const syscore_mq_attr_t *attr, syscore_mq_attr_t *old_attr);
```
- **Purpose**: Sets new attributes on the queue (only flags can be modified).

---

## Memory Mapping

Abstractions wrapping virtual memory page allocations and page protections.

| Function | Description |
|---|---|
| `syscore_mmap` | Maps files or devices into the calling process's address space. |
| `syscore_munmap` | Deletes the mappings for the specified address range. |
| `syscore_mprotect` | Changes access protections for the calling process's memory pages. |
| `syscore_msync` | Flushes changes made to in-memory copy of a file back to disk. |

### syscore_mmap

```c
syscore_error_t syscore_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset, void **out_addr);
```
- **Purpose**: Maps files or devices into the calling process's virtual address space.
- **Parameters**:
  - `addr`: Starting address (use `NULL` for system default placement).
  - `length`: Mapping range size in bytes.
  - `prot`: Access protections (`SYSCORE_PROT_READ`, `SYSCORE_PROT_WRITE`, etc.).
  - `flags`: Visibility options (`SYSCORE_MAP_SHARED`, `SYSCORE_MAP_PRIVATE`, `SYSCORE_MAP_ANONYMOUS`).
  - `fd`: Open file descriptor (use `-1` for anonymous memory mappings).
  - `offset`: File offset (use `0` for anonymous mappings).
  - `out_addr`: Output pointer to receive the mapped virtual memory address.

### syscore_munmap

```c
syscore_error_t syscore_munmap(void *addr, size_t length);
```
- **Purpose**: Deletes the mappings for the specified address range.

### syscore_mprotect

```c
syscore_error_t syscore_mprotect(void *addr, size_t length, int prot);
```
- **Purpose**: Changes access protections for the calling process's memory pages.

### syscore_msync

```c
syscore_error_t syscore_msync(void *addr, size_t length, int flags);
```
- **Purpose**: Flushes changes made to in-memory copy of a file back to disk.

# Error Codes

SysCore uses a unified error enum mapping system errors into portable codes:

| Error Code | Numerical Value | Typical Cause |
|---|---|---|
| `SYSCORE_SUCCESS` | `0` | Operation completed successfully. |
| `SYSCORE_ERROR_GENERIC` | `-1` | An unspecified OS-level error occurred. |
| `SYSCORE_ERROR_INVALID_ARGUMENT` | `-2` | A provided parameter is out of bounds or `NULL`. |
| `SYSCORE_ERROR_OUT_OF_MEMORY` | `-3` | The system was unable to allocate memory. |
| `SYSCORE_ERROR_NOT_FOUND` | `-4` | The requested file, handle, or path was not found. |
| `SYSCORE_ERROR_PERMISSION_DENIED` | `-5` | The calling process lacks necessary credentials. |
| `SYSCORE_ERROR_TIMEOUT` | `-6` | A blocking call timed out. |
| `SYSCORE_ERROR_NOT_SUPPORTED` | `-7` | The operation is not supported on the target platform. |
| `SYSCORE_ERROR_IO` | `-8` | An I/O error occurred during system read/write. |
| `SYSCORE_ERROR_BUSY` | `-9` | The target resource is locked or currently in use. |

# Usage Examples

### Creating a Process
[basic_fork.c](../examples/process/basic_fork.c):
```c
syscore_pid_t pid;
syscore_error_t err = syscore_process_fork(&pid);
if (err == SYSCORE_SUCCESS) {
    if (pid == 0) {
        printf("Child process\n");
    } else {
        int status;
        syscore_process_wait(pid, &status, 0);
    }
}
```

### Creating a Thread
[basic_thread.c](../examples/threading/basic_thread.c):
```c
syscore_thread_t thread;
syscore_thread_create(&thread, NULL, thread_func, NULL);
syscore_thread_join(thread, NULL);
```

### Using Mutexes
[mutex_demo.c](../examples/sync/mutex_demo.c):
```c
syscore_mutex_t mutex;
syscore_mutex_init(&mutex);
syscore_mutex_lock(&mutex);
// Critical section
syscore_mutex_unlock(&mutex);
syscore_mutex_destroy(&mutex);
```

### Shared Memory
[shared_memory_demo.c](../examples/memory/shared_memory_demo.c):
```c
syscore_shm_handle_t shm_fd;
syscore_shm_create("/demo_shm", 256, 0666, &shm_fd);
void *addr;
syscore_shm_map(shm_fd, 256, 1, &addr);
syscore_shm_unmap(addr, 256);
syscore_shm_close(shm_fd);
syscore_shm_destroy("/demo_shm");
```

### Message Queues
[message_queue_demo.c](../examples/ipc/message_queue_demo.c):
```c
syscore_mq_handle_t mq;
syscore_mq_open("/demo_mq", 2, 0, 0666, NULL, &mq);
syscore_mq_send(mq, "msg", 4, 1);
syscore_mq_close(mq);
syscore_mq_unlink("/demo_mq");
```

### Memory Mapping
[mmap_anonymous_demo.c](../examples/memory/mmap_anonymous_demo.c):
```c
void *addr;
syscore_mmap(NULL, 4096, SYSCORE_PROT_READ | SYSCORE_PROT_WRITE,
             SYSCORE_MAP_PRIVATE | SYSCORE_MAP_ANONYMOUS, -1, 0, &addr);
syscore_munmap(addr, 4096);
```

# Best Practices

### Checking Return Values
Verify the `syscore_error_t` return status. You can use the `SYSCORE_RETURN_IF_ERROR(expr)` macro to log and propagate failures:
```c
syscore_error_t err = syscore_mutex_lock(&mutex);
if (err != SYSCORE_SUCCESS) {
    // Handle mutex locking error
}
```

### Cleaning Up Resources
Ensure resource allocation structures are cleaned up symmetrically. If an operation fails mid-way through execution, clean up previously allocated handles (such as unmapping memory blocks and closing file descriptors) before exiting the function scope.

### Destroying Synchronization Primitives
Never attempt to call `_destroy()` functions (e.g. `syscore_mutex_destroy()`) on primitives that are locked, in use, or have threads waiting on them.

### Closing File Descriptors
Symmetrically close all open file handles using `syscore_ipc_close()` or `syscore_shm_close()` respectively. Neglecting to close handles leads to file descriptor leakage, which can exhaust the process's descriptor table limits.

### Portability
Prefer SysCore APIs over direct POSIX calls to maintain portability across supported platforms.

# API Stability

### Current Version
The current stable library release is version `1.0.0`.

### API Stability Expectations
The public API is intended to remain source compatible throughout the current major release whenever practical. Any addition of future modules or features will be implemented as non-breaking extensions.
