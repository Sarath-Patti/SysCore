# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

### Changed

### Fixed

### Removed

## [1.0.0] - 2026-07-19

### Added
- Modular C17 systems programming library providing portable POSIX platform service abstractions.
- Process Management module (`syscore_process_fork`, `syscore_process_exec`, `syscore_process_wait`, PID/PPID getters).
- IPC module providing anonymous pipes (`syscore_ipc_pipe_create`) and named FIFOs (`syscore_ipc_fifo_create`, `syscore_ipc_fifo_open`).
- Threading module supporting POSIX thread creation, joining, sleep helpers, and thread attribute configuration.
- Synchronization module wrapping mutexes (`syscore_mutex_t`), condition variables (`syscore_cond_t`), and read-write locks (`syscore_rwlock_t`).
- Shared Memory module managing POSIX shared memory objects, virtual address space mapping, unmapping, and unlinking.
- Semaphores module supporting unnamed and named POSIX semaphores with fallback emulation on macOS.
- Message Queue module providing priority message queue operations with fallback shared memory/semaphore emulation on macOS.
- Memory Mapping module wrapping POSIX `mmap`, `munmap`, `mprotect`, and `msync`.
- Cross-platform support for Ubuntu Linux (GCC) and macOS (AppleClang).
- CMake build system with target installation support (`cmake --install`) and exported package targets (`SysCoreConfig.cmake`).
- Automated CTest integration suite with 9 unit test executables.
- Benchmark suite covering thread, pipe, semaphore, and memory mapping latency.
- 21 example programs demonstrating subsystem APIs.
- GitHub Actions CI pipeline running automated builds and tests on Linux and macOS.

### Documentation
- Completed comprehensive documentation suite:
  - `README.md`: Project overview, features table, quick start, architecture diagram, and repository layout.
  - `docs/Architecture.md`: Subsystem layers, module organization, error propagation, and cross-platform design.
  - `docs/API.md`: Public API reference with function prototypes, parameters, error codes, and examples.
  - `docs/Build.md`: CMake build instructions, testing driver commands, benchmark targets, and troubleshooting guide.
  - `docs/Examples.md`: Catalog and walkthrough for all example programs under `examples/`.
  - `docs/Portability.md`: Platform compatibility guide covering native Linux wrappers and macOS fallback emulation.
  - `docs/Roadmap.md`: Chronological milestones, version 1.0 highlights, and future development directions.
  - `docs/Performance.md`: Latency benchmark methodology, benchmark suite details, and execution guidelines.
  - `CONTRIBUTING.md`: Contributor guide outlining environment setup, workflow, coding standards, and PR submission rules.

### Testing
- Automated unit test suite integrated with CTest validating error propagation, handle lifecycles, and concurrency primitives across Linux and macOS environments.

### CI/CD
- GitHub Actions CI pipeline executing automated builds and unit test suites on `ubuntu-latest` and `macos-latest` workflows.

### Notes
- Initial stable release of the SysCore systems programming library.
