# Performance Overview

This document describes the performance evaluation methodology, benchmark programs, and execution guidelines for the SysCore systems library.

### Purpose of the Benchmark Suite
The SysCore benchmark suite provides automated tools to measure the latency overhead of core operating system abstractions wrapped by the library. Rather than evaluating application throughput, the benchmarks measure execution delays for fundamental system operations such as thread lifecycle management, IPC channel transfers, semaphore signaling, and virtual memory page allocations.

### Importance of Benchmarking
Low-level systems programming libraries must minimize wrapper overhead over underlying POSIX system calls. Benchmarking allows maintainers and developers to:
- Evaluate the execution delay introduced by SysCore wrapper layers.
- Compare performance trends across different target operating systems (Linux vs. macOS).
- Detect performance regressions during codebase evolution.

### Relationship Between Testing and Benchmarking
- **Automated Unit Testing** (`tests/`): Verifies functional correctness, error propagation, and API contracts.
- **Performance Benchmarking** (`benchmarks/`): Measures execution time and operational latency over repeated loop iterations under controlled conditions.

# Benchmark Suite

The repository contains four dedicated latency benchmark programs under the `benchmarks/` directory:

| Benchmark File | Module Exercised | Operation Measured | Iterations |
|---|---|---|---|
| `thread_latency.c` | Threading (`syscore_threading`) | Sequential thread creation and joining latency (`syscore_thread_create` + `syscore_thread_join`) | 1,000 |
| `pipe_latency.c` | IPC (`syscore_ipc`) | Round-trip byte write and read latency over an anonymous pipe (`syscore_ipc_write` + `syscore_ipc_read`) | 10,000 |
| `semaphore_latency.c` | Synchronization (`syscore_semaphore`) | Unnamed semaphore lock and unlock cycle latency (`syscore_sem_wait` + `syscore_sem_post`) | 100,000 |
| `mmap_latency.c` | Memory Mapping (`syscore_mmap`) | Virtual memory page allocation and unmapping latency (`syscore_mmap` + `syscore_munmap`) | 5,000 |

### Detailed Subsystem Measurements

- **Thread Spawn/Join Latency** (`thread_latency.c`): Measures the average time required to allocate, spawn, execute, and join a minimal execution thread.
- **Pipe Round-Trip Latency** (`pipe_latency.c`): Measures the time to write a single byte payload to a pipe descriptor and read it back from the read descriptor.
- **Semaphore Wait/Post Latency** (`semaphore_latency.c`): Measures the overhead of acquiring and releasing a non-blocking/blocking semaphore barrier in a loop.
- **Mmap/Munmap Page Latency** (`mmap_latency.c`): Measures the time to map a single page (4096 bytes) of private anonymous memory and immediately unmap it.

# Benchmark Methodology

SysCore benchmarks use a standardized measurement approach:

- **Monotonic High-Resolution Timers**: Elapsed time is recorded using POSIX `clock_gettime(CLOCK_MONOTONIC, ...)` around tight execution loops to eliminate system clock adjustments.
- **Repeated Measurements**: Operations are executed repeatedly over fixed iteration counts (from 1,000 to 100,000 iterations) to amortize timer resolution overhead and compute arithmetic mean latency per operation in microseconds ($\mu\text{s}$).
- **Release Build Selection**: Benchmarks must be compiled in `Release` configuration (`-DCMAKE_BUILD_TYPE=Release`) so compiler optimization flags (`-O3`) are active and logging overhead is minimized.
- **Environmental Variations**: Execution results vary depending on host hardware, CPU frequency scaling, operating system kernel scheduler policies, and background process activity.

# Running Benchmarks

### Building Benchmarks
Compile the benchmark targets using CMake in Release mode:

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Executing Benchmark Binaries
Run the compiled benchmark executables directly from the build output directory:

```bash
./build/thread_latency
./build/pipe_latency
./build/semaphore_latency
./build/mmap_latency
```

### Interpreting Console Output
Each benchmark outputs its calculated per-operation arithmetic mean latency upon completion:

```text
Thread Spawn/Join Latency: XX.XXX us
Pipe Round-Trip Latency: X.XXX us
Semaphore Wait/Post Loop Latency: X.XXX us
Mmap/Munmap Page Latency: X.XXX us
```

# Measuring Performance

The benchmark suite focuses strictly on measuring individual low-level operation latencies:

- **Thread Lifecycle**: Measures thread allocation and kernel context teardown cost.
- **IPC Data Transfer**: Measures context switching and buffer copying between kernel pipe descriptors.
- **Synchronization Primitives**: Measures user-space lock acquisition and atomic state transitions.
- **Virtual Page Management**: Measures operating system virtual memory page table manipulation and TLB invalidation overhead.

# Factors Affecting Performance

Several system-level factors influence benchmark measurements:

| Factor | Impact on Performance |
|---|---|
| **CPU Architecture** | Core clock frequency, L1/L2 cache capacity, and microarchitectural branch prediction affect loop execution rates. |
| **OS Kernel Scheduler** | Context switch latency, process scheduling priorities, and kernel lock contention alter IPC and thread results. |
| **Compiler Optimizations** | `Release` mode (`-O3`) enables function inlining, instruction reordering, and loop unrolling, significantly outperforming unoptimized `Debug` builds. |
| **Background System Load** | Concurrent process execution on the host machine increases cache eviction rates and scheduling jitter. |
| **Platform Compatibility Layers** | Fallback emulation paths on macOS (such as named-semaphore based message queues) introduce different performance characteristics compared to native Linux syscalls. |

# Interpreting Results

When analyzing SysCore benchmark output:

- **Relative Comparison**: Compare benchmark measurements across identical host environments or across code changes rather than relying on absolute numbers.
- **Trend Evaluation**: Evaluate performance trends over time to identify unintended performance regressions introduced by codebase changes.
- **Environmental Context**: Always record the host OS, compiler version, build type, and hardware specifications when saving benchmark runs for comparison.

# Best Practices

- **Always Use Release Builds**: Never evaluate performance on `Debug` builds.
- **Quiesce the Environment**: Close unnecessary background applications and background processes before running benchmark programs.
- **Run Multiple Iterations**: Execute benchmark binaries multiple times and calculate baseline averages to filter out transient system spikes.
- **Compare Equivalent Environments**: Perform comparative evaluations on identical hardware platforms and operating system versions.
- **Relative Measurement Only**: Treat benchmark metrics as relative indicators of wrapper efficiency rather than hard performance guarantees.

# Future Benchmark Expansion

As SysCore expands, future benchmark programs may be added to measure latency and throughput overhead for newly introduced subsystems (such as socket connection setup, event loop demultiplexing, and signal handling).
