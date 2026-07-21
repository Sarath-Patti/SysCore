# Architecture Overview

SysCore is a modular C systems programming library designed to provide unified, standard abstractions for POSIX platform services. By wrapping raw operating system calls in consistent APIs with strict error checking, SysCore simplifies process control, thread synchronization, and virtual memory management in Unix-like environments.

The primary design goals of the SysCore library are:
- **Modularity**: Loose coupling and high cohesion across platform subsystems.
- **Portability**: Uniform compilation and execution behavior on Linux and macOS hosts.
- **Safety**: Safe handling of buffers, defensive argument validations, and clean resource mapping.
- **API Consistency**: Symmetric parameter ordering, error code propagation, and clean handle representations.

A modular architecture was selected to isolate platform-specific details. Subsystems can implement custom emulation layers (such as macOS message queue emulation) without exposing internal mechanisms or breaking the library's public interfaces.

# System Layers

The SysCore system design is organized into layered tiers. Each tier communicates only with its adjacent lower layer to ensure logical isolation:

```
                  +-----------------------------------+
                  |        Client Applications        |
                  +-----------------+-----------------+
                                    |
                                    v
                  +-----------------+-----------------+
                  |        SysCore Public APIs        |
                  +-----------------+-----------------+
                                    |
     +------------------------------+------------------------------+
     |                              |                              |
     v                              v                              v
+----+---------+               +----+---------+               +----+---------+
|   Process    |               |  IPC & Msg   |               | Memory &     |
|  Management  |               |    Queues    |               | Mappings     |
+----+---------+               +----+---------+               +----+---------+
     |                              |                              |
     v                              v                              v
+----+---------+               +----+---------+               +----+---------+
|  Threading   |               |  Mutexes &   |               | Shared Mem & |
|  & Control   |               |  Locks       |               | Semaphores   |
+----+---------+               +----+---------+               +----+---------+
     |                              |                              |
     +------------------------------+------------------------------+
                                    |
                                    v
                  +-----------------+-----------------+
                  |      Operating System Services    |
                  |           (Linux / macOS)         |
                  +-----------------------------------+
```

- **Client Applications**: Call public APIs to request process, synchronization, memory, or IPC actions.
- **SysCore Public APIs**: Expose unified function interfaces and type handles prefixed with `syscore_`.
- **Subsystem Modules**: Implement wrappers and platform emulation engines.
- **POSIX Operating System**: Underlying kernel interfaces (system calls) executed by the compiler runtime (glibc or Darwin SDK).

# Module Organization

The codebase is organized into nine discrete modules.

---

### Common Module
- **Purpose**: Provides logging, error definitions, and compile-time utility metadata.
- **Responsibilities**: Maps system `errno` values to `syscore_error_t` codes, records structured messages with logging levels, and manages version identifiers.
- **Public Interfaces**: [version.h](../include/common/version.h), [logging.h](../include/common/logging.h), [errors.h](../include/common/errors.h).
- **Internal Implementation**: Wraps custom formatted strings with levels (`DEBUG`, `INFO`, `WARN`, `ERROR`), printing with optional ANSI color codes.
- **Dependencies**: Standard library headers only.

---

### Process Module
- **Purpose**: Abstracts native operating system processes.
- **Responsibilities**: Spawns child processes, replaces process execution images, coordinates parent-child waits, and queries process identifiers.
- **Public Interfaces**: [process.h](../include/process/process.h).
- **Internal Implementation**: Direct wrappers around POSIX `fork()`, `execvp()`, and `waitpid()`.
- **Dependencies**: Common.

---

### IPC Module
- **Purpose**: Handles basic inter-process communication channels.
- **Responsibilities**: Manages anonymous pipes and named FIFOs, providing clean synchronous read/write helper hooks.
- **Public Interfaces**: [ipc.h](../include/ipc/ipc.h).
- **Internal Implementation**: Wraps POSIX `pipe()`, `mkfifo()`, `open()`, `read()`, and `write()`.
- **Dependencies**: Common.

---

### Threading Module
- **Purpose**: Spawns and manages independent execution threads.
- **Responsibilities**: Handles thread creation, joining, precise sleep delays, and customization of stack or detach attributes.
- **Public Interfaces**: [threading.h](../include/threading/threading.h).
- **Internal Implementation**: Wraps standard `pthread_create()`, `pthread_join()`, `pthread_attr_init()`, and POSIX `nanosleep()` system calls.
- **Dependencies**: Common.

---

### Synchronization Module
- **Purpose**: Coordinates concurrent thread execution.
- **Responsibilities**: Manages mutual exclusion regions, wait condition variables, and reader-writer locking.
- **Public Interfaces**: [sync.h](../include/sync/sync.h).
- **Internal Implementation**: Abstractions over standard POSIX threading mutexes (`pthread_mutex_t`), condition variables (`pthread_cond_t`), and read-write locks (`pthread_rwlock_t`).
- **Dependencies**: Common, Threading.

---

### Shared Memory Module
- **Purpose**: Shares raw physical memory blocks across processes.
- **Responsibilities**: Creates, maps, and unmaps shared memory segments.
- **Public Interfaces**: [shared_memory.h](../include/memory/shared_memory.h).
- **Internal Implementation**: Wraps POSIX shared memory APIs (`shm_open()`, `ftruncate()`, `mmap()`, `munmap()`, `shm_unlink()`).
- **Dependencies**: Common.

---

### Semaphores Module
- **Purpose**: Controls concurrent thread and process resource access boundaries.
- **Responsibilities**: Supports unnamed (memory-backed) and named (system-wide) semaphores.
- **Public Interfaces**: [semaphore.h](../include/sync/semaphore.h).
- **Internal Implementation**: 
  - **Linux**: Direct wrappers for `sem_init()`, `sem_wait()`, `sem_post()`, and `sem_destroy()`.
  - **macOS**: Emulates unnamed semaphores internally using unique, dynamically allocated named semaphores to bypass Apple's deprecation warnings.
- **Dependencies**: Common.

---

### Message Queues Module
- **Purpose**: Manages structured inter-process message pass channels.
- **Responsibilities**: Exposes message queuing with custom priorities, queue sizes, and non-blocking parameters.
- **Public Interfaces**: [message_queue.h](../include/ipc/message_queue.h).
- **Internal Implementation**:
  - **Linux**: Standard POSIX `<mqueue.h>` wrapper functions.
  - **macOS**: Custom emulation layer constructed using Shared Memory blocks and Semaphores, ensuring identical API behaviors. Bounded object names are generated via deterministic `djb2_hash` algorithms to fit macOS 31-character limits.
- **Dependencies**: Common, Shared Memory, Semaphores.

---

### Memory Mapping Module
- **Purpose**: Manages virtual memory page mappings.
- **Responsibilities**: Supports anonymous/file-backed, shared/private, read-only/read-write memory projections, page protection edits, and disk flushes.
- **Public Interfaces**: [mmap.h](../include/memory/mmap.h).
- **Internal Implementation**: Wraps POSIX `mmap()`, `munmap()`, `mprotect()`, and `msync()` calls.
- **Dependencies**: Common.

# Error Handling

SysCore enforces a defensive, crash-free error management methodology:

- **Unified Error Codes**: Most functions return a `syscore_error_t` code (e.g. `SYSCORE_SUCCESS`, `SYSCORE_ERROR_INVALID_ARGUMENT`, `SYSCORE_ERROR_IO`). Outputs are returned through caller-provided pointers as the final arguments.
- **Logging Subsystem**: High-visibility logging is implemented with levels `DEBUG`, `INFO`, `WARN`, and `ERROR`. Internal failures map `errno` values and print descriptive messages via logging macros before propagating errors up.
- **Error Propagation**: Caller validations and system call failures utilize propagation macros (e.g., `SYSCORE_RETURN_IF_ERROR`) to bubble error states back cleanly to parent layers.
- **Resource Cleanup**: Subsystem layers ensure strict resources cleanups (e.g., closing file descriptors or unlinking semaphores) on error exit paths before returning, eliminating resource leaks.

# Repository Organization

The repository layout follows professional open-source conventions:

- `include/`: Houses all public C header files exposing types, handles, and signatures.
- `src/`: Houses all internal `.c` implementation source files, isolating platform-specific logic.
- `examples/`: Provides clean example programs illustrating typical workflows.
- `tests/`: Contains module-specific automated test programs managed via the CTest engine.
- `benchmarks/`: Holds latency measuring scripts to track operation execution overhead.
- `docs/`: Stores architecture descriptions, API references, portability tables, and development guides.

# Cross-Platform Design

To prevent polluting public headers with preprocessor macros, SysCore enforces a clean POSIX abstraction strategy:

- **Linux Implementation**: Directly leverages native GNU Libc (glibc) system interfaces. Compiles under strict C17 standards with GCC and Clang compilers.
- **macOS Compatibility**: Apple macOS limits or deprecates several standard POSIX systems (such as unnamed semaphores and standard POSIX message queues). The library handles this internally within the implementation source files:
  - macOS unnamed semaphores are emulated under the hood using unique named semaphores (`/sc_sem_<pid>_<address>`) that are closed and unlinked upon destruction.
  - POSIX Message Queues are emulated on macOS using a shared memory ring buffer orchestrated by named semaphores. All resources use hashed paths (DJB2) under 18 characters to comply with Apple's 31-character naming constraint.
- **Abstraction Design**: All compiler guards and `#ifdef` conditionals are kept inside `.c` source files. Public headers remain clean and identical for both platforms, preserving binary compatibility.

# Design Principles

The framework conforms to the following software engineering design principles:

- **Modularity**: Subsystems are decoupled, defining clear functional boundaries.
- **Encapsulation**: Details of platform-specific types (such as custom macOS queue structures) are hidden inside the implementation source files, exposing opaque types or handles to client code.
- **Consistent APIs**: All APIs follow a uniform naming standard (`syscore_<module>_<action>`), consistent parameter orders, and return `syscore_error_t`.
- **Defensive Programming**: Function entries validate input pointers and boundary parameters, systems calls check return codes, and read/write buffers are safely null-terminated.
- **Resource Ownership**: Subsystems define symmetric allocations and deallocations APIs (e.g., `syscore_shm_create` and `syscore_shm_destroy`), ensuring callers have distinct lifecycle ownership.
- **Portability**: Uses standard C17 syntax and POSIX feature macros, ensuring compiler warning-free compilation on all compliant toolchains.

# Future Evolution

The architecture is designed to accommodate future extensions without requiring structural changes to existing public APIs:
- **Networking**: Will integrate as a dedicated socket subsystem wrapping TCP and UDP stream APIs.
- **Event Loop**: Will be built as an asynchronous multiplexer mapping to `epoll` (Linux) and `kqueue` (macOS).
- **Signals & Timers**: Will introduce interceptor hooks and thread-based callback timer queues.
- **Thread Pool**: Will reside above the threading and synchronization modules, scheduling client jobs to pre-spawned worker threads.
