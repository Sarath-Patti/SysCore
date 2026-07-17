# SysCore

SysCore is a modular, production-quality systems programming framework.

## Project Structure

- `include/`: Public headers (e.g., `include/common/`, `include/process/`).
- `src/`: Source files (e.g., `src/common/`, `src/process/`).
- `examples/`: Example usage and demos (e.g., `examples/process/`).
- `tests/`: Unit and integration tests.
- `benchmarks/`: Performance benchmarks.
- `docs/`: Design and API documentation.
- `scripts/`: Development and build helper scripts.

## Version

Current version: 0.3.0 (Milestone: v0.3 – Process Management)

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

## Building

This project uses CMake. To configure and build:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Running the Demos

After building, you can run the primary demo or process examples:

```bash
# Main Infrastructure Demo
./syscore_demo

# Process Management Demo
./process_demo
```

