# Portability Overview

SysCore provides a modular, platform-independent systems programming library. By defining consistent public APIs and wrapping system interfaces internally, SysCore enables developers to write applications that compile and run across multiple POSIX host environments.

Portability is essential for building robust systems utilities, integration tests, and application backends that deploy seamlessly. SysCore hides compiler flags, platform macros, and operating system API gaps from user code, maintaining absolute consistency in public function signatures and error propagation.

# Supported Platforms

The library is verified to compile warning-free and execute correctly on the following systems:

| Platform | Compiler | Status | Notes |
|---|---|---|---|
| **Ubuntu Linux** | GCC 9+ / Clang 10+ | Fully Supported | Uses native Linux POSIX system interfaces. |
| **macOS** | AppleClang 12+ | Fully Supported | Direct fallback emulation layers for semaphores and message queues. |

# Design Philosophy

SysCore's architecture enforces strict platform abstraction rules:

- **Platform-Independent Public APIs**: Public header files inside `include/` contain only standard function declarations, type handles, and generic enums. No platform-specific headers or conditional preprocessor switches are exposed to the user.
- **Hidden Implementations**: Platform-specific headers, conditional compilation statements (such as `#if defined(...)`), and emulation structures are confined strictly inside the `.c` source files under `src/`.
- **Minimal Conditional Compilation**: SysCore aims to keep code paths unified. Platform-specific execution is modularized into discrete internal helper routines rather than scattered throughout the codebase.
- **Consistent Behavior**: The library standardizes return values, error classifications, and operational traits (such as priority ordering or non-blocking parameters) across all supported operating systems.

# Platform Differences

---

## Linux

On Linux platforms, SysCore maps directly to standard native POSIX system interfaces provided by GNU Libc (glibc):

- **Message Queues**: Wraps native Linux `<mqueue.h>` functions (`mq_open`, `mq_send`, `mq_receive`, etc.), linking with the realtime library (`-lrt`) if required by the toolchain.
- **Unnamed Semaphores**: Uses native `<semaphore.h>` unnamed semaphore functions (`sem_init`, `sem_destroy`, `sem_wait`, `sem_post`).
- **Shared Memory**: Utilizes standard POSIX shared memory objects (`shm_open`, `ftruncate`, `shm_unlink`) to allocate shared memory regions.
- **Memory Mapping**: Maps directly to native virtual memory mapping APIs (`mmap`, `munmap`, `mprotect`, `msync`).

---

## macOS

macOS (Darwin kernel) deviates from standard POSIX specifications in several subsystems. SysCore implements internal compatibility and emulation layers to address these limitations:

- **Unnamed Semaphore Emulation**: macOS deprecates `sem_init` and `sem_destroy`, producing compile-time warnings that violate strict build policies. SysCore emulates unnamed semaphores on macOS by allocating unique named semaphores (`/sc_sem_<pid>_<address>`) internally. These are opened, closed, and unlinked automatically during the structure's lifecycle.
- **POSIX Message Queue Emulation**: macOS does not implement POSIX message queues (`<mqueue.h>`). SysCore emulates message queues on macOS using shared memory objects as ring buffers, synchronized by named semaphores.
- **Naming Constraints**: Darwin restricts named semaphores and shared memory object paths to a maximum of 31 characters. To prevent "File name too long" errors (`errno 63`), SysCore hashes the queue and semaphore identifiers deterministically (using `djb2_hash`), producing unique names under 18 characters (e.g. `/sc_mq_s_%08x`).
- **Shared Memory & Memory Mapping**: Uses Darwin's standard POSIX shared memory objects and virtual memory mappings (`shm_open`, `mmap`, `munmap`, `mprotect`, `msync`).

# Internal Compatibility Strategy

- **Platform-Independent Headers**: Type handles (such as `syscore_mq_handle_t` or `syscore_sem_t`) are declared as opaque structures or handles, hiding platform-specific pointer variables.
- **Platform-Specific Logic**: Confined to internal source folders (`src/sync/semaphore.c`, `src/ipc/message_queue.c`).
- **Consistent Error Reporting**: Translates Linux and macOS error codes to unified `syscore_error_t` values.
- **Symmetric Resource Management**: Ensures temporary emulation resources (e.g., named semaphores, shared files) are unlinked and cleaned up during failure paths and destruction loops.

# Compiler Support

SysCore requires a standard compiler supporting the C17 standard (`-std=c17`) and POSIX.1-2008 specifications:

- **GCC**: Verified under Linux systems.
- **AppleClang**: Verified under macOS platforms.

All targets compile warning-free with strict compiler options enabled (`-Wall -Wextra -Wpedantic -Werror`).

# Portability Guidelines

To write portable concurrent code, adhere to the following recommendations:

- **Prefer SysCore APIs**: Avoid calling native POSIX interfaces directly (e.g., use `syscore_sem_init` instead of `sem_init`). This ensures compatibility on macOS.
- **Avoid Platform-Specific Assumptions**: Do not assume message queues (`<mqueue.h>`) are supported natively, or thatUnnamed semaphores are implemented directly.
- **Check Return Values**: Always verify the `syscore_error_t` status for failures.
- **Clean Up Resources**: Ensure open handles are closed, memory mapped regions are unmapped, and named resources are unlinked symmetrically.
- **Use Documented Interfaces Only**: Do not modify or rely on the private structure members defined in opaque handles.

# Known Limitations

- **Resource Representation on macOS**: Emulated unnamed semaphores and message queues on macOS consume Darwin named semaphore and shared memory object slots, which are visible in the filesystem.
- **macOS Name Limits**: User-supplied message queue and semaphore names must comply with standard POSIX conventions. Internally, SysCore hashes these names to fit within Darwin's 31-character limit.

# Future Expansion

To add support for a new operating system (e.g., FreeBSD):

1. Expose the same headers inside the `include/` directory.
2. In the corresponding `.c` implementation files under `src/`, add conditional compilation blocks for the new platform (e.g. `#elif defined(SYSCORE_OS_FREEBSD)`).
3. Map SysCore functions to the new platform's system APIs.
4. Define error code mappings from the platform's `errno` values to `syscore_error_t` to maintain consistent behavior.
