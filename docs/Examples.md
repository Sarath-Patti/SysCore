# Examples Overview

The `examples/` directory contains demonstration programs that show how to use each SysCore module. These examples are designed to illustrate correct API usage, error handling, resource lifecycle management, and common concurrent systems programming patterns.

### Recommended Learning Order
To understand the library's design and progression, we recommend studying the examples in the following order:
1. **Process Management**: Basic child process creation.
2. **Threading**: Thread spawning, parameter passing, and joining.
3. **Synchronization**: Mutual exclusion, thread signaling, and locking boundaries.
4. **Inter-Process Communication**: Pipe, FIFO, and queue communications.
5. **Shared Memory**: Multi-process memory block mapping and semaphores coordination.
6. **Memory Mapping**: Virtual page mapping protection and file-backed synchronization.

# Running Examples

All example binaries are compiled automatically during the standard CMake build step.

### Compiling
To build all examples:
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Execution
The compiled binaries are placed directly under the `build/` directory. Execute them from the workspace root:
```bash
./build/basic_fork
./build/anonymous_pipe_demo
./build/named_fifo_demo
./build/basic_thread
./build/mutex_demo
./build/shared_memory_demo
./build/message_queue_demo
./build/mmap_anonymous_demo
```

# Example Categories

---

## Process Management

Abstractions for system process creation and waiting.

- **File Name**: `basic_fork.c` (source: [basic_fork.c](../examples/process/basic_fork.c))
- **Purpose**: Demonstrates process duplication and waiting for child process exit status.
- **APIs Demonstrated**: `syscore_process_fork`, `syscore_process_wait`, `syscore_process_get_pid`, `syscore_process_get_ppid`.
- **Expected Behavior**: Spawns a child process. The child prints its identifier and terminates. The parent waits, reads the child's exit status (`42`), and prints a completion log message.

---

## Inter-Process Communication

This category covers three communication abstractions: anonymous pipes, named FIFOs, and message queues.

### Anonymous Pipes
- **File Name**: `anonymous_pipe.c` (source: [anonymous_pipe.c](../examples/ipc/anonymous_pipe.c))
- **Purpose**: Implements unidirectional inter-process communication using anonymous pipes.
- **APIs Demonstrated**: `syscore_ipc_pipe_create`, `syscore_ipc_read`, `syscore_ipc_write`, `syscore_ipc_close`.
- **Expected Behavior**: Parent creates a pipe and duplicates itself. The parent writes a string payload into the pipe. The child reads the payload, outputs the message, and both parent and child close their respective handle descriptors.

### Named FIFOs
- **File Name**: `named_fifo.c` (source: [named_fifo.c](../examples/ipc/named_fifo.c))
- **Purpose**: Connects processes using a filesystem-backed named FIFO special file.
- **APIs Demonstrated**: `syscore_ipc_fifo_create`, `syscore_ipc_fifo_open`, `syscore_ipc_read`, `syscore_ipc_write`, `syscore_ipc_close`.
- **Expected Behavior**: Creates a FIFO file at `/tmp/syscore_demo_fifo`. The child opens it read-only. The parent sleeps briefly, opens it write-only, and writes a message. The child reads and outputs the string. The parent waits for the child and unlinks the FIFO file.

### Message Queues
- **File Name**: `message_queue_demo.c` (source: [message_queue_demo.c](../examples/ipc/message_queue_demo.c))
  - **Purpose**: Demonstrates basic message queue configuration, sending, and receiving.
  - **APIs Demonstrated**: `syscore_mq_open`, `syscore_mq_send`, `syscore_mq_receive`, `syscore_mq_close`, `syscore_mq_unlink`.
  - **Expected Behavior**: Child opens a queue, writes a priority message, and exits. Parent receives the message, closes the handle, and unlinks the queue.
- **File Name**: `message_queue_priority.c` (source: [message_queue_priority.c](../examples/ipc/message_queue_priority.c))
  - **Purpose**: Demonstrates priority-sorted extraction.
  - **APIs Demonstrated**: `syscore_mq_open`, `syscore_mq_send`, `syscore_mq_receive`, `syscore_mq_close`, `syscore_mq_unlink`.
  - **Expected Behavior**: Sends three messages with priorities `5`, `20`, and `10` respectively. Receives them in priority-sorted order (`20`, `10`, `5`).
- **File Name**: `producer_consumer_mq.c` (source: [producer_consumer_mq.c](../examples/ipc/producer_consumer_mq.c))
  - **Purpose**: Coordinates a concurrent producer/consumer loop.
  - **APIs Demonstrated**: `syscore_mq_open`, `syscore_mq_send`, `syscore_mq_receive`, `syscore_mq_close`, `syscore_mq_unlink`.
  - **Expected Behavior**: Spawns a producer child that pushes 5 item messages into a queue. The parent consumes and logs each item.

---

## Threading

Spawning and joining threads of execution.

- **File Name**: `basic_thread.c` (source: [basic_thread.c](../examples/threading/basic_thread.c))
  - **Purpose**: Shows thread spawn and wait integration.
  - **APIs Demonstrated**: `syscore_thread_create`, `syscore_thread_join`, `syscore_thread_sleep_ms`.
  - **Expected Behavior**: Spawns a thread, prints status logs, sleeps for 100ms, and joins execution back to the main thread.
- **File Name**: `multiple_threads.c` (source: [multiple_threads.c](../examples/threading/multiple_threads.c))
  - **Purpose**: Manages multiple concurrent threads.
  - **APIs Demonstrated**: `syscore_thread_create`, `syscore_thread_join`.
  - **Expected Behavior**: Spawns three worker threads that print identifiers and wait, main thread blocks until all three join.
- **File Name**: `thread_arguments.c` (source: [thread_arguments.c](../examples/threading/thread_arguments.c))
  - **Purpose**: Passes structured arguments to execution threads.
  - **APIs Demonstrated**: `syscore_thread_create`, `syscore_thread_join`.
  - **Expected Behavior**: Passes custom structs to two threads, threads execute computations using the struct members and log results.

---

## Synchronization

Standard thread mutual exclusion, condition variables signaling, and locks coordination.

- **Mutexes**: `mutex_demo.c` (source: [mutex_demo.c](../examples/sync/mutex_demo.c))
  - **Purpose**: Protects a shared counter from concurrent data races.
  - **APIs Demonstrated**: `syscore_mutex_init`, `syscore_mutex_lock`, `syscore_mutex_unlock`, `syscore_mutex_destroy`.
  - **Expected Behavior**: Spawns two threads incrementing a shared counter; mutex locks serialize updates to ensure the final counter matches exactly.
- **Condition Variables**: `condition_variable.c` (source: [condition_variable.c](../examples/sync/condition_variable.c))
  - **Purpose**: Renders state-based worker blocking and signaling.
  - **APIs Demonstrated**: `syscore_cond_init`, `syscore_cond_wait`, `syscore_cond_signal`, `syscore_cond_destroy`, `syscore_mutex_lock`, `syscore_mutex_unlock`.
  - **Expected Behavior**: Consumer thread blocks on a condition variable until a producer thread sets a flag and signals it to wake up.
- **Reader/Writer Locks**: `read_write_lock.c` (source: [read_write_lock.c](../examples/sync/read_write_lock.c))
  - **Purpose**: Allows concurrent read access while ensuring exclusive write locks.
  - **APIs Demonstrated**: `syscore_rwlock_init`, `syscore_rwlock_rdlock`, `syscore_rwlock_wrlock`, `syscore_rwlock_unlock`, `syscore_rwlock_destroy`.
  - **Expected Behavior**: Multiple reader threads acquire shared locks concurrently. A writer thread blocks until readers release their locks, acquiring exclusive write permissions.
- **Semaphores**:
  - `semaphore_demo.c` (source: [semaphore_demo.c](../examples/sync/semaphore_demo.c)):
    - **Purpose**: Basic unnamed semaphore wait/post execution.
    - **APIs Demonstrated**: `syscore_sem_init`, `syscore_sem_wait`, `syscore_sem_post`, `syscore_sem_destroy`.
    - **Expected Behavior**: Worker thread blocks waiting for main thread post notifications.
  - `named_semaphore_demo.c` (source: [named_semaphore_demo.c](../examples/sync/named_semaphore_demo.c)):
    - **Purpose**: Cross-process unnamed semaphore emulation utilizing system-wide named paths.
    - **APIs Demonstrated**: `syscore_sem_open`, `syscore_sem_close`, `syscore_sem_unlink`, `syscore_sem_wait`, `syscore_sem_post`.
    - **Expected Behavior**: Child blocks waiting for a system-wide named semaphore. The parent sleeps, posts a signal, waits for the child to exit, and unlinks the semaphore path.

---

## Shared Memory

Shared memory objects allocations and virtual mapping regions.

- **File Name**: `shared_memory_demo.c` (source: [shared_memory_demo.c](../examples/memory/shared_memory_demo.c))
  - **Purpose**: Maps shared memory objects to read and write data across process limits.
  - **APIs Demonstrated**: `syscore_shm_create`, `syscore_shm_open`, `syscore_shm_map`, `syscore_shm_unmap`, `syscore_shm_close`, `syscore_shm_destroy`.
  - **Expected Behavior**: Parent creates shared memory. The child maps it read-only and waits. Parent maps it read-write and copies a string. The child reads the mapped virtual address region, logs the string, and unmaps.
- **File Name**: `shared_counter.c` (source: [shared_counter.c](../examples/memory/shared_counter.c))
  - **Purpose**: Coordinates concurrent updates to a counter stored in a shared memory region.
  - **APIs Demonstrated**: `syscore_shm_create`, `syscore_shm_open`, `syscore_shm_map`, `syscore_sem_init`, `syscore_sem_wait`, `syscore_sem_post`, `syscore_sem_destroy`.
  - **Expected Behavior**: Parent and child processes map a shared state structure. Both safely increment a shared counter variable, using a shared unnamed semaphore initialized in the region to serialize accesses.

---

## Memory Mapping

Abstractions wrapping virtual memory page allocations and page protections.

- **File Name**: `mmap_anonymous_demo.c` (source: [mmap_anonymous_demo.c](../examples/memory/mmap_anonymous_demo.c))
  - **Purpose**: Demonstrates anonymous virtual mappings, changing protection flags to Read-Only, and unmapping.
  - **APIs Demonstrated**: `syscore_mmap`, `syscore_munmap`, `syscore_mprotect`.
  - **Expected Behavior**: Allocates anonymous memory pages, writes a string, uses `mprotect` to change permissions to Read-Only, verifies reading works, and unmaps.
- **File Name**: `mmap_file_demo.c` (source: [mmap_file_demo.c](../examples/memory/mmap_file_demo.c))
  - **Purpose**: Maps a file descriptor to a virtual region, edits bytes, and flushes modifications back to disk.
  - **APIs Demonstrated**: `syscore_mmap`, `syscore_munmap`, `syscore_msync`.
  - **Expected Behavior**: Creates a file, maps it read-write, directly edits bytes in the memory map region, calls `msync` to synchronize disk, unmaps, and reads back the file to verify modifications.
- **File Name**: `mmap_shared_demo.c` (source: [mmap_shared_demo.c](../examples/memory/mmap_shared_demo.c))
  - **Purpose**: Shares a file-backed virtual mapping between parent and child processes.
  - **APIs Demonstrated**: `syscore_mmap`, `syscore_munmap`, `syscore_process_fork`, `syscore_process_wait`.
  - **Expected Behavior**: Maps a shared file, forks, child writes a string into the mapped region, parent waits and reads back the updated string.

# Learning Path

We recommend studying the modules in this progression:

1. **Process**: Understand child process lifecycle boundaries (`basic_fork.c`). This sets the base for multi-process environments.
2. **Threading**: Learn thread execution pathways (`basic_thread.c`, `multiple_threads.c`).
3. **Synchronization**: Implement serialization checks (`mutex_demo.c`, `read_write_lock.c`) to coordinate concurrent access.
4. **IPC**: Implement unidirectional data transfers (`anonymous_pipe.c`, `named_fifo.c`).
5. **Shared Memory**: Implement fast, shared memory mapping exchanges (`shared_memory_demo.c`, `shared_counter.c`) using semaphores for synchronization.
6. **Message Queues**: Integrate priority-sorted messaging queues (`message_queue_demo.c`, `message_queue_priority.c`).
7. **Memory Mapping**: Master page alignments, protection modifications, and disk flushes (`mmap_file_demo.c`).

This progression moves from basic execution isolation to advanced, coordinated inter-process resource sharing.

# Common Patterns

Each example follows a consistent lifecycle pattern:

```
[ Initialize Resources ]
        │
        ▼
[ Validate Return Code ] ──(Failure)──► [ Clean up & Exit ]
        │
    (Success)
        ▼
[ Perform Operations ]
        │
        ▼
[ Symmetrical Cleanup ]
```

1. **Initialize Resources**: Open descriptors, map virtual addresses, or initialize locks (e.g. `syscore_shm_create`, `syscore_mutex_init`).
2. **Validate Return Code**: Verify if `syscore_error_t == SYSCORE_SUCCESS` before proceeding.
3. **Perform Operations**: Execute core business logic (e.g. read/write, fork, sync).
4. **Symmetrical Cleanup**: Release mappings, unlink paths, and close handles before exiting (e.g. `syscore_munmap`, `syscore_ipc_close`).

# Best Practices

- **Study Examples First**: Review the example implementations to understand correct handle and argument usage.
- **Check Return Values**: Never ignore error codes. Use propagation macros to catch runtime errors.
- **Clean Up Symmetrically**: Ensure every resource allocation function has a matching cleanup call on all execution paths.
- **Prefer SysCore APIs**: Avoid direct OS/POSIX calls where SysCore wrapper equivalents exist to maintain portability.
- **Run Demos on All Platforms**: Build and run example targets on both Linux and macOS to verify portability.
