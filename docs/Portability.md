# SysCore Portability & Platform Support

SysCore supports Linux and macOS.

## Platform Differences

### macOS POSIX Message Queue Emulation
Apple macOS does not support standard `<mqueue.h>` APIs. To bypass this, the SysCore Message Queue module employs a custom emulation engine constructed using:
- **Shared Memory**: Utilized to buffer message payloads.
- **Named Semaphores**: Utilized to orchestrate blocking/non-blocking coordination across processes.

All internal macOS object paths are bounded via deterministic name hashing (DJB2) to comply with Apple's 31-character Named Semaphore path limit.

### macOS Semaphore Emulation
`sem_init` and `sem_destroy` are deprecated on macOS. On macOS, SysCore emulates unnamed semaphores using unique named semaphores (`sem_open` / `sem_close` / `sem_unlink`) under the hood to ensure warnings-free compilation.
