# SysCore Phase 4 — Performance Analysis & Comparative Experiments

This document describes the design, methodology, architectural considerations, and measured findings of **Phase 4: Performance Analysis & Comparative Experiments** in SysCore.

---

## 1. Overview & Objectives

The goal of Phase 4 is to transform SysCore's microbenchmarking infrastructure into a controlled performance-analysis framework. Rather than evaluating primitives in isolation, Phase 4 provides:
1. **IPC Comparison**: Direct latency, throughput, and payload-scaling evaluation across Pipe, Shared Memory, and POSIX Message Queue (64B, 256B, 1024B payloads).
2. **Synchronization Comparison**: Direct comparison across Mutex, Semaphore, Read-Write Lock, and Condition Variable under identical concurrency regimes (1, 2, 4, 8 threads).
3. **Concurrency Scaling Analysis**: Quantitative measurement of throughput scaling and scaling efficiency ($E_N = \frac{T_N}{N \cdot T_1}$).
4. **Resource & CPU Profiling**: Cross-platform system and user CPU time tracking via standard POSIX `getrusage(RUSAGE_SELF, ...)` with graceful fallback.
5. **Controlled Baseline vs. Optimized Experiment**: Empirical validation of lock amortization (byte-by-byte locked transfer vs. block-amortized `memcpy` transfer under identical 4 KB workloads).
6. **Machine-Readable Result Generation**: Automated generation of `build/experiments_results.json` and `build/experiments_results.csv`.

---

## 2. Experimental Methodology

### Workload Definitions
- **IPC Payload Transfer**: High-frequency payload transmission between producer/consumer or writer/reader ends using pipes, mutex-protected shared memory, or POSIX message queues.
- **Synchronization Contention**: Concurrent atomic counter increment and lock acquisition/release across 1, 2, 4, and 8 worker threads.
- **Locked Memory Transfer**: Transfer of a 4096-byte memory payload into a destination buffer under mutual exclusion.

### Measurement Methodology
- **Warm-Up Phase**: Each benchmark executes non-measured warm-up iterations (100–500 iterations) to prime CPU caches, page tables, and branch predictors.
- **Timing & Statistics**: High-resolution POSIX `clock_gettime(CLOCK_MONOTONIC, ...)` captures start and end timestamps for every measured iteration. Statistical summaries include arithmetic mean, minimum, maximum, median (p50), p95, p99, and sample standard deviation.
- **Resource Measurements**: Process user CPU time (`ru_utime`) and system CPU time (`ru_stime`) are recorded using `getrusage(RUSAGE_SELF, ...)` around each measurement window.

### Platform Differences
- **macOS (Darwin)**: High-resolution monotonic timer via `CLOCK_MONOTONIC`. POSIX Message Queues (`mq_open`, `mq_send`, `mq_receive`) and Semaphores are supported via Darwin POSIX emulation layers.
- **Linux**: High-performance POSIX timers (`CLOCK_MONOTONIC`) and native futex-backed pthread synchronization primitives and `/dev/mqueue` message queues. Requires linking `-lrt` for real-time IPC extensions.

---

## 3. How to Reproduce Experiments

To build SysCore and execute the complete Phase 4 experiment suite:

```bash
# 1. Configure and Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 2. Run All Automated CTest Tests
ctest --test-dir build --output-on-failure

# 3. Execute Phase 4 Performance Analysis Suite
python3 scripts/run_experiments.py
```

After execution, the following comparative result artifacts are generated in `build/`:
- `build/experiments_results.json`: Complete machine-readable JSON dataset containing all statistical percentiles, configuration metadata, and CPU usage.
- `build/experiments_results.csv`: Flattened CSV table suitable for data analysis, charting, or spreadsheet import.

To verify Phase 3 performance regression detection remains passing:
```bash
python3 scripts/compare_benchmarks.py
```

---

## 4. Measured Experimental Findings

> **Note**: All numerical values below are populated directly from executed empirical benchmarks on the target system.

### 4.1 IPC Performance Comparison

| Mechanism | Payload Size | Avg Latency | p50 Latency | p95 Latency | Throughput | Total CPU Time |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Shared Memory** | 64 Bytes | 91.00 ns | 0.00 ns | 1.00 µs | 8.36 M ops/s | 0.000363 s |
| **Pipe** | 64 Bytes | 1.39 µs | 1.00 µs | 2.00 µs | 691.09 K ops/s | 0.004313 s |
| **POSIX Message Queue** | 64 Bytes | 2.82 µs | 2.00 µs | 5.00 µs | 349.49 K ops/s | 0.004280 s |
| **Shared Memory** | 256 Bytes | 38.67 ns | 0.00 ns | 0.00 ns | 15.62 M ops/s | 0.000191 s |
| **Pipe** | 256 Bytes | 534.00 ns | 1.00 µs | 1.00 µs | 1.79 M ops/s | 0.001674 s |
| **POSIX Message Queue** | 256 Bytes | 1.94 µs | 2.00 µs | 2.00 µs | 508.82 K ops/s | 0.002949 s |
| **Shared Memory** | 1024 Bytes | 57.33 ns | 0.00 ns | 1.00 µs | 12.10 M ops/s | 0.000249 s |
| **Pipe** | 1024 Bytes | 637.67 ns | 1.00 µs | 1.00 µs | 1.51 M ops/s | 0.001987 s |
| **POSIX Message Queue** | 1024 Bytes | 2.01 µs | 2.00 µs | 2.00 µs | 491.32 K ops/s | 0.003054 s |

**Observations**:
- Shared memory avoids kernel context switching during data transfer, achieving nanosecond-range latencies (38.67 ns - 91.00 ns) and order-of-magnitude higher throughput than kernel-buffered mechanisms.
- Pipes incur kernel buffer copy overhead, resulting in sub-microsecond to microsecond latencies (534 ns - 1.39 µs).
- POSIX Message Queues enforce structured message boundary management and priority queue ordering, resulting in higher fixed overhead (1.94 µs - 2.82 µs avg latency).

---

### 4.2 Synchronization & Concurrency Scaling Comparison

| Primitive | Concurrency | Avg Latency | p50 Latency | Throughput | Scaling Efficiency ($E_N$) | Total CPU Time |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Mutex** | 1 Thread | 39.00 ns | 0.00 ns | 13.70 M ops/s | 100.0% | 0.000223 s |
| **RWLock** | 1 Thread | 40.33 ns | 0.00 ns | 12.93 M ops/s | 100.0% | 0.000234 s |
| **Semaphore** | 1 Thread | 950.00 ns | 1.00 µs | 1.02 M ops/s | 100.0% | 0.002953 s |
| **Condition Variable**| 1 Thread | 16.43 µs | 16.00 µs | 60.73 K ops/s | 100.0% | 0.029292 s |
| **Mutex** | 2 Threads | 21.57 µs | 20.00 µs | 46.26 K ops/s | 0.2% | 0.094031 s |
| **RWLock** | 2 Threads | 22.09 µs | 21.00 µs | 45.17 K ops/s | 0.2% | 0.097793 s |
| **Semaphore** | 2 Threads | 56.37 µs | 58.00 µs | 17.73 K ops/s | 0.9% | 0.202131 s |
| **Condition Variable**| 2 Threads | 23.18 µs | 22.00 µs | 43.05 K ops/s | 35.4% | 0.052366 s |
| **Mutex** | 4 Threads | 29.41 µs | 27.00 µs | 33.96 K ops/s | 0.1% | 0.027994 s |
| **RWLock** | 4 Threads | 30.41 µs | 28.00 µs | 32.84 K ops/s | 0.1% | 0.028506 s |
| **Semaphore** | 4 Threads | 103.45 µs | 103.00 µs | 9.66 K ops/s | 0.2% | 0.077332 s |
| **Condition Variable**| 4 Threads | 27.02 µs | 24.00 µs | 36.98 K ops/s | 15.2% | 0.015412 s |
| **Mutex** | 8 Threads | 56.27 µs | 52.00 µs | 17.76 K ops/s | 0.0% | 0.057559 s |
| **RWLock** | 8 Threads | 56.51 µs | 52.00 µs | 17.68 K ops/s | 0.0% | 0.059166 s |
| **Semaphore** | 8 Threads | 201.92 µs | 197.00 µs | 4.95 K ops/s | 0.1% | 0.161279 s |
| **Condition Variable**| 8 Threads | 50.53 µs | 50.00 µs | 19.77 K ops/s | 4.1% | 0.031346 s |

**Observations**:
- Uncontended Mutex and RWLock operations complete in ~39–40 ns.
- As thread contention increases from 1 to 8 threads, lock serialization and thread context switching overhead dominate execution time, reducing effective scaling efficiency.

---

### 4.3 Controlled Optimization Experiment (Byte-by-Byte vs. Block-Amortized Transfer)

- **Workload**: Transfer of a 4096-byte payload under mutual exclusion.
- **Baseline Implementation**: Unamortized transfer acquiring and releasing the lock 4,096 times (once per byte transferred).
- **Optimized Implementation**: Amortized block transfer acquiring and releasing the lock once per 4,096-byte block.

| Implementation Version | Workload | Avg Latency | Throughput | Total CPU Time | Measured Relative Change |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Baseline (Unamortized Locked)** | 4096B Transfer | 33.72 µs | 29.63 K ops/s | 0.101215 s | 0.00% (Reference) |
| **Optimized (Block-Amortized)** | 4096B Transfer | 93.00 ns | 8.52 M ops/s | 0.000354 s | **+28,659.09%** |

**Observations**:
- Amortizing lock acquisition across block transfers reduces per-operation mutex synchronization overhead by 4096x.
- Latency drops from 33.72 µs to 93.00 ns, yielding a measured throughput increase of **+28,659.09%** and reducing Total CPU time from 0.101s to 0.00035s.

---

## 5. Conclusion

Phase 4 establishes a quantitative performance analysis pipeline for SysCore. By combining high-resolution benchmarking, resource monitoring (`getrusage`), structured JSON/CSV export, and automated scaling analysis, SysCore provides empirical evidence for systems architecture decisions.
