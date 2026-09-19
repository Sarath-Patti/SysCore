# SysCore

[![CI Build](https://github.com/Sarath-Patti/SysCore/actions/workflows/ci.yml/badge.svg)](https://github.com/Sarath-Patti/SysCore/actions)
![License](https://img.shields.io/github/license/Sarath-Patti/SysCore)
![C Standard](https://img.shields.io/badge/C-17-blue)
![Platforms](https://img.shields.io/badge/platforms-Linux%20%7C%20macOS-lightgrey)
![Version](https://img.shields.io/badge/version-v1.0.0-blue)

SysCore is a modular C systems programming library that provides unified abstractions for POSIX platform services. By wrapping raw operating system calls in consistent APIs with explicit error propagation, SysCore simplifies process control, thread synchronization, and memory mapping on Unix-like environments.

The library is designed with a focus on cross-platform portability, modular architecture, and API consistency. It compiles warning-free under strict standards on both Linux and macOS systems, utilizing custom emulation layers where native features differ.

Developers can leverage SysCore as a lightweight systems engineering layer for system utilities, test integration code, and concurrent application services requiring explicit error management and predictable resource cleanups.

## Project Overview

| Attribute | Value |
|---|---|
| Language | C |
| Standard | C17 |
| Build System | CMake (3.15+) |
| Platforms | Linux, macOS |
| Modules | 10 (Common, Process, IPC, Threading, Synchronization, Shared Memory, Semaphores, Message Queues, Memory Mapping, Benchmark) |
| Example Programs | 21 |
| Unit Tests | 10 (CTest Integration) |
| Benchmarks | 13 |
| CI | GitHub Actions |
| License | MIT |

## Quick Start

Get started by cloning the repository and running the automated build and test targets:

```bash
git clone https://github.com/Sarath-Patti/SysCore.git
cd SysCore
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Key Features

| Feature | Description | Abstractions & APIs |
|---|---|---|
| **Process Management** | Process spawning, execution, state waiting, and metadata lookup. | `syscore_process_fork`, `syscore_process_exec`, `syscore_process_wait`, PID / PPID getters |
| **IPC** | Unidirectional local channels and named pipes. | Anonymous pipes, named FIFOs, unified read/write wrappers |
| **Threading** | OS-independent POSIX threads with attribute setup options. | `syscore_thread_create`, `syscore_thread_join`, `syscore_thread_sleep_ms`, thread stack and detach configurations |
| **Synchronization** | Mutual exclusion boundaries and concurrent thread signaling. | Mutexes (`syscore_mutex_t`), condition variables (`syscore_cond_t`), read-write locks (`syscore_rwlock_t`) |
| **Shared Memory** | Memory sharing across process limits. | POSIX Shared Memory segments (`shm_open`, `ftruncate`, `mmap`, `munmap`) |
| **POSIX Semaphores** | Value barriers with portable macOS fallback implementations. | Unnamed and named semaphore wrappers |
| **POSIX Message Queues** | Priority-ordered blocking messaging queues. | Message queue attributes, priority ordering, blocking/non-blocking modes, and custom macOS emulation |
| **Memory Mapping** | Page-aligned virtual memory block allocations and protections. | Anonymous and file-backed mappings, memory protection modification (`mprotect`), and disk synchronization (`msync`) |
| **Performance Engineering** | Reusable high-resolution microbenchmarking framework. | `syscore_benchmark_run`, `syscore_benchmark_config_t`, setup/step/teardown lifecycle, percentiles ($p_{50}$, $p_{95}$, $p_{99}$) |
| **Cross-platform Support** | Clean compilation across multiple target platforms. | Verified warning-free builds on Ubuntu (GCC) and macOS (AppleClang) |
| **CMake Build System** | Modern library target installation and integration definitions. | Target installation rules and CMake export file targets |
| **Automated Testing** | Core library unit testing infrastructure. | Automated test suite integrated with CTest runner |

## Architecture

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

## Repository Structure

```
SysCore/
├── README.md             # Project landing documentation
├── LICENSE               # License terms file
├── CHANGELOG.md          # History of milestones changes
├── CMakeLists.txt        # Root build configuration script
├── cmake/                # Auxiliary CMake files
├── include/              # Public API definition headers
│   ├── benchmark/        # Microbenchmarking framework headers
│   ├── common/           # Central options, errors, logging configurations
│   ├── ipc/              # Pipes, message queues
│   ├── memory/           # Shared memory segment, mmap
│   ├── process/          # Process fork and exec wrappers
│   └── sync/             # Mutexes, semaphores, condvars
├── src/                  # Library internal implementations source files
├── examples/             # Subsystem example code and demos
├── tests/                # Custom unit test executables
└── benchmarks/           # Performance benchmark executables
```

## Building & Installation

To build all library targets, examples, tests, and benchmarks, use CMake:

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

To install the compiled SysCore libraries and public headers globally on the system:

```bash
cmake --install build --prefix /usr/local
```

This registers the headers and generates standard target configuration modules (`SysCoreConfig.cmake` and `SysCoreTargets.cmake`) for consumers to find and link.

## Running Examples

Example executables are placed in the `build/` directory. Run representative examples:

```bash
# Main infrastructure and logging demo
./build/syscore_demo

# Process management demo
./build/basic_fork

# IPC anonymous pipes demo
./build/anonymous_pipe_demo

# IPC named FIFO demo
./build/named_fifo_demo

# Threading and control demo
./build/basic_thread

# Mutex synchronization demo
./build/mutex_demo

# Shared memory segment demo
./build/shared_memory_demo

# POSIX message queue demo
./build/message_queue_demo

# Memory mapping (mmap) demo
./build/mmap_anonymous_demo
```

## Testing

SysCore contains a full suite of automated unit tests covering all library modules and the microbenchmarking framework (`test_benchmark`). The build status and test correctness are verified continuously on Ubuntu (GCC) and macOS (AppleClang) environments using GitHub Actions and CTest.

Run the test suite locally using the following command:

```bash
cd build
ctest --output-on-failure
```

## Performance Engineering & Benchmarking

SysCore provides a reusable, high-resolution microbenchmarking framework (`syscore_benchmark`) designed to systematically measure the performance characteristics of SysCore's IPC and synchronization mechanisms under controlled workloads and concurrency levels.

### Framework Features

- **Monotonic High-Resolution Timing:** Uses OS-level monotonic clocks (`CLOCK_MONOTONIC`) to avoid time skew artifacts.
- **Explicit Lifecycle Hooks:** Setup and teardown functions execute outside timed iteration blocks unless lifecycle cost itself is being explicitly measured.
- **Configurable Iterations:** Configurable warm-up and measurement iteration loops per benchmark run.
- **Comprehensive Statistical Reporting:** Reports total elapsed time, throughput (ops/sec), average latency, minimum, maximum, median ($p_{50}$), $p_{95}$, $p_{99}$ percentiles, and standard deviation.

### Benchmark Executables

#### Phase 1 Microbenchmarks
Evaluates baseline operational overheads for core system primitives:
- `mutex_latency`: Uncontended mutex lock/unlock latency.
- `semaphore_latency`: Uncontended semaphore wait/post execution times.
- `thread_latency`: Thread creation (`syscore_thread_create`) and joining (`syscore_thread_join`) overhead.
- `pipe_latency`: Round-trip pipe communication latency.
- `shm_latency`: Shared memory segment access latency.
- `mmap_latency`: Virtual memory mapping (`mmap`/`munmap`) allocation rates.

#### Phase 2 IPC Performance Suite
Evaluates communication performance across payload sizes and thread counts:
- `ipc_pipe_bench`: Pipe communication throughput and latency across varying payload sizes (1 B, 64 B, 256 B, 1024 B).
- `ipc_shm_bench`: Shared memory access latency under multi-threaded contention (1, 2, 4, 8 threads).
- `ipc_mq_bench`: POSIX message queue send/receive latency and throughput (64 B, 256 B payload sizes).

#### Phase 2 Synchronization Performance Suite
Evaluates synchronization primitive performance under controlled concurrency and workload patterns:
- `sync_mutex_bench`: Mutex lock/unlock latency uncontended (1 thread) and under active multi-threaded contention (2, 4, 8 threads).
- `sync_semaphore_bench`: Semaphore wait/post latency uncontended (1 thread) and under active multi-threaded contention (2, 4, 8 threads).
- `sync_rwlock_bench`: Read-write lock performance across baseline single-reader, read-heavy (8 readers / 1 writer), write-heavy (1 reader / 4 writers), and mixed (4 readers / 4 writers) workloads.
- `sync_condvar_bench`: Condition variable producer-consumer wait/signal synchronization cost (`syscore_cond_wait`/`syscore_cond_signal`).

### Running Benchmarks

Build all benchmark executables in Release mode:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run Phase 1 microbenchmarks:
```bash
./build/mutex_latency
./build/semaphore_latency
./build/thread_latency
./build/pipe_latency
./build/shm_latency
./build/mmap_latency
```

Run Phase 2 IPC and Synchronization benchmark suites:
```bash
./build/ipc_pipe_bench
./build/ipc_shm_bench
./build/ipc_mq_bench
./build/sync_mutex_bench
./build/sync_semaphore_bench
./build/sync_rwlock_bench
./build/sync_condvar_bench
```

### Design & Measurement Notes

- **Isolated Timed Regions:** Setup and teardown run outside measured iteration loops so initialization costs do not distort operation timings.
- **Explicit Workloads:** Workload parameters (payload size, concurrency level, operation count) are explicitly declared per benchmark configuration.
- **Cross-Platform Compatibility:** Benchmarks run on both Linux and macOS. On macOS, POSIX message queues utilize SysCore's platform-specific shared-memory emulation layer.
- **Hardware & OS Dependencies:** Benchmark metrics are system-dependent and influenced by CPU architecture, OS thread scheduling, and compiler optimizations. Results should be evaluated under controlled, consistent environments.

## Documentation

For specific subsystems and design implementation details, refer to the documents in the `docs/` directory:

- [Architecture.md](docs/Architecture.md): Structural layout and internal error propagation rules.
- [Build.md](docs/Build.md): Compilation configurations and system deployment instructions.
- [API.md](docs/API.md): Detailed signatures and documentation for all public APIs.
- [Examples.md](docs/Examples.md): Detailed mapping and walkthrough of example code.
- [Portability.md](docs/Portability.md): Notes on target dependencies and custom emulation implementations.
- [Roadmap.md](docs/Roadmap.md): Future project goals.

## Supported Platforms

| Platform | Compiler | Support Status | Notes |
|---|---|---|---|
| **Ubuntu Linux** | GCC 9+ / Clang 10+ | Fully Supported | Direct mapping to native Linux POSIX APIs. |
| **macOS** | AppleClang 12+ | Fully Supported | Direct fallback emulation layers for semaphores and message queues. |

## Continuous Integration

Every commit and pull request targeted to the `main` branch is validated automatically via a GitHub Actions CI pipeline. The pipeline runs compilation targets on Linux and macOS host platforms and executes all unit tests to confirm zero build errors or test failures.

## Roadmap

### Completed Features (v1.0.0)
- Unified logging and error handling configurations.
- Native process management and IPC pipe abstractions.
- POSIX threading, mutexes, condvars, and rwlocks.
- Shared memory and semaphore wraps (including macOS compatibility emulation).
- POSIX message queues with priority sorting.
- Virtual memory mappings (`mmap`, `msync`, `mprotect`).
- High-resolution microbenchmarking framework (`syscore_benchmark`) and IPC / synchronization performance suites.
- Custom test suites, benchmarks, and target installation rules.

### Planned Features
- **Networking**: High-level TCP and UDP socket connection wrappers.
- **Event Loop**: Epoll/kqueue backed event demultiplexer.
- **Signals**: Unified POSIX signal interceptors and hooks.
- **Timers**: Portable high-resolution timer queues.
- **Thread Pool**: Scalable worker thread pool configuration.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
