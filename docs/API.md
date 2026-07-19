# SysCore Public API Reference

## Common Infrastructure
- `syscore_get_version_string(void)`
- `syscore_error_string(syscore_error_t err)`

## Process Management
- `syscore_process_fork(syscore_pid_t *out_pid)`
- `syscore_process_exec(const char *path, char *const argv[])`
- `syscore_process_wait(syscore_pid_t pid, int *status, int options)`
- `syscore_process_get_pid(void)`
- `syscore_process_get_ppid(void)`

## IPC Modules
- `syscore_ipc_pipe_create(syscore_ipc_handle_t *rd, syscore_ipc_handle_t *wr)`
- `syscore_ipc_fifo_create(const char *path, int mode)`
- `syscore_ipc_fifo_open(const char *path, int write_mode, int non_block, syscore_ipc_handle_t *out)`
- `syscore_ipc_read(syscore_ipc_handle_t handle, void *buf, size_t count, size_t *out_read)`
- `syscore_ipc_write(syscore_ipc_handle_t handle, const void *buf, size_t count, size_t *out_written)`
- `syscore_ipc_close(syscore_ipc_handle_t handle)`

## Threading & Sync
- `syscore_thread_create(syscore_thread_t *thread, const syscore_thread_attr_t *attr, void *(*func)(void*), void *arg)`
- `syscore_thread_join(syscore_thread_t thread, void **retval)`
- `syscore_thread_self(void)`
- `syscore_thread_sleep_ms(unsigned int ms)`
- `syscore_mutex_init(syscore_mutex_t *mutex)`
- `syscore_mutex_lock(syscore_mutex_t *mutex)`
- `syscore_mutex_unlock(syscore_mutex_t *mutex)`
- `syscore_mutex_destroy(syscore_mutex_t *mutex)`

## Memory Mapping & Queues
- `syscore_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset, void **out_addr)`
- `syscore_munmap(void *addr, size_t length)`
- `syscore_mq_open(const char *name, int mode, int non_block, int permissions, const syscore_mq_attr_t *attr, syscore_mq_handle_t *out)`
