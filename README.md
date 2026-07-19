# SysCore

SysCore is a modular, production-quality systems programming framework.

## Project Structure

- `include/`: Public headers (e.g., `include/common/`, `include/process/`, `include/ipc/`, `include/threading/`, `include/sync/`, `include/memory/`).
- `src/`: Source files (e.g., `src/common/`, `src/process/`, `src/ipc/`, `src/threading/`, `src/sync/`, `src/memory/`).
- `examples/`: Example usage and demos (e.g., `examples/process/`, `examples/ipc/`, `examples/threading/`, `examples/sync/`, `examples/memory/`).
- `tests/`: Unit and integration tests.
- `benchmarks/`: Performance benchmarks.
- `docs/`: Design and API documentation.
- `scripts/`: Development and build helper scripts.

## Version

Current version: 0.8.0 (Milestone: v0.8 – POSIX Message Queues)

## Capabilities

1. **Common Infrastructure**
   - Centralized project configurations and versioning.
   - Comprehensive OS and Compiler detection helpers.
   - Lightweight logging system supporting levels `DEBUG`, `INFO`, `WARN`, `ERROR` with optional ANSI colors.
   - Crash-free error handling system (`syscore_error_t` and `SYSCORE_RETURN_IF_ERROR`).
2. **Process Management**
   - Reusable process management interface using POSIX APIs.
   - Wrapper for fork (`syscore_process_fork`).
   - Wrapper for program execution (`syscore_process_exec`).
   - Wrapper for waiting child status (`syscore_process_wait`).
   - Helper to retrieve current and parent PID (`syscore_process_get_pid`, `syscore_process_get_ppid`).
3. **Pipes & IPC**
   - Lightweight POSIX-compliant IPC module.
   - Anonymous pipe creation (`syscore_ipc_pipe_create`) and handle closure (`syscore_ipc_close`).
   - Named FIFO creation (`syscore_ipc_fifo_create`) and opening (`syscore_ipc_fifo_open`).
   - Reusable read/write handlers (`syscore_ipc_read`, `syscore_ipc_write`).
4. **POSIX Threads**
   - Reusable POSIX threading module.
   - Thread creation (`syscore_thread_create`) and joining (`syscore_thread_join`).
   - Retrieve current thread (`syscore_thread_self`).
   - Precise millisecond sleep helpers (`syscore_thread_sleep_ms`).
   - Thread stack and detach attribute customization support (`syscore_thread_attr_t`).
5. **Synchronization**
   - POSIX mutex wrappers (`syscore_mutex_t`) for mutual exclusion.
   - POSIX condition variables (`syscore_cond_t`) for thread coordination.
   - POSIX read-write locks (`syscore_rwlock_t`) for multiple readers/single writer concurrency.
6. **Shared Memory & Semaphores**
   - POSIX Shared Memory management (`shm_open`, `shm_unlink`, `ftruncate`, `mmap`, `munmap`, `close`).
   - POSIX Semaphores wrapper supporting unnamed (`sem_init`, `sem_destroy`) and named (`sem_open`, `sem_close`, `sem_unlink`) semaphores.
7. **POSIX Message Queues**
   - POSIX Message Queue wrappers (`mq_open`, `mq_close`, `mq_unlink`, `mq_send`, `mq_receive`, `mq_getattr`, `mq_setattr`).
   - macOS compatibility emulation using shared memory and semaphores.

## Building

This project uses CMake. To configure and build:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Running the Demos

After building, you can run the primary demo, process examples, IPC examples, threading examples, synchronization examples, memory/semaphore examples, or message queue examples:

```bash
# Main Infrastructure Demo
./syscore_demo

# Process Management Demo
./basic_fork

# IPC Anonymous Pipe Demo
./anonymous_pipe_demo

# IPC Named FIFO Demo
./named_fifo_demo

# POSIX Threads Demos
./basic_thread
./thread_arguments
./multiple_threads

# Synchronization Demos
./mutex_demo
./producer_consumer
./read_write_lock
./condition_variable

# Shared Memory & Semaphores Demos
./shared_memory_demo
./semaphore_demo
./named_semaphore_demo
./shared_counter

# POSIX Message Queue Demos
./message_queue_demo
./message_queue_priority
./producer_consumer_mq
```






