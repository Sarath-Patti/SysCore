#!/usr/bin/env python3
"""
SysCore Benchmark Runner
Executes all SysCore benchmark executables and collects machine-readable JSON results.
"""

import argparse
import os
import subprocess
import sys

BENCHMARK_EXECUTABLES = [
    "mutex_latency",
    "semaphore_latency",
    "thread_latency",
    "pipe_latency",
    "shm_latency",
    "mmap_latency",
    "ipc_pipe_bench",
    "ipc_shm_bench",
    "ipc_mq_bench",
    "sync_mutex_bench",
    "sync_semaphore_bench",
    "sync_rwlock_bench",
    "sync_condvar_bench",
]

def main():
    parser = argparse.ArgumentParser(description="Run SysCore Performance Benchmarks")
    parser.add_argument("--build-dir", default="build", help="Path to build directory (default: build)")
    parser.add_argument("--output", default="build/results.json", help="Path to output JSON file (default: build/results.json)")
    args = parser.parse_args()

    build_dir = os.path.abspath(args.build_dir)
    output_path = os.path.abspath(args.output)

    if not os.path.exists(build_dir):
        print(f"Error: Build directory '{build_dir}' does not exist.", file=sys.stderr)
        sys.exit(1)

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    if os.path.exists(output_path):
        os.remove(output_path)

    env = os.environ.copy()
    env["SYSCORE_BENCHMARK_JSON_FILE"] = output_path

    print(f"Executing {len(BENCHMARK_EXECUTABLES)} benchmark suites...")
    print(f"Machine-readable output destination: {output_path}\n")

    failed_count = 0
    for bench_name in BENCHMARK_EXECUTABLES:
        exe_path = os.path.join(build_dir, bench_name)
        if not os.path.exists(exe_path):
            print(f"Warning: Benchmark executable '{exe_path}' not found. Skipping.", file=sys.stderr)
            failed_count += 1
            continue

        print(f"-> Running {bench_name}...")
        res = subprocess.run([exe_path], env=env)
        if res.returncode != 0:
            print(f"Error: Benchmark '{bench_name}' failed with exit code {res.returncode}.", file=sys.stderr)
            failed_count += 1

    if os.path.exists(output_path):
        print(f"\nSuccessfully generated benchmark results at: {output_path}")
    else:
        print(f"\nError: Output file '{output_path}' was not generated.", file=sys.stderr)
        sys.exit(1)

    sys.exit(failed_count)

if __name__ == "__main__":
    main()
