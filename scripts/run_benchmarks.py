#!/usr/bin/env python3
"""
SysCore Benchmark Runner
Executes all SysCore benchmark executables and collects machine-readable JSON results.
Supports multi-pass execution and representative median run selection across runs.
"""

import argparse
import json
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

def run_single_pass(build_dir, pass_output_path):
    if os.path.exists(pass_output_path):
        os.remove(pass_output_path)

    env = os.environ.copy()
    env["SYSCORE_BENCHMARK_JSON_FILE"] = pass_output_path

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

    return failed_count

def aggregate_results(pass_files, final_output_path):
    all_runs_data = []
    for pfile in pass_files:
        if not os.path.exists(pfile):
            print(f"Error: Pass file '{pfile}' was not found.", file=sys.stderr)
            sys.exit(1)
        try:
            with open(pfile, "r", encoding="utf-8") as f:
                data = json.load(f)
                if isinstance(data, list):
                    all_runs_data.append(data)
                else:
                    print(f"Error: Pass file '{pfile}' JSON is not a list.", file=sys.stderr)
                    sys.exit(1)
        except Exception as e:
            print(f"Error loading pass file '{pfile}': {e}", file=sys.stderr)
            sys.exit(1)

    expected_runs = len(pass_files)
    if len(all_runs_data) != expected_runs:
        print(f"Error: Expected {expected_runs} pass data files, but loaded {len(all_runs_data)}.", file=sys.stderr)
        sys.exit(1)

    config_order = []
    configs_map = {}

    for run_data in all_runs_data:
        for item in run_data:
            if not isinstance(item, dict) or "name" not in item:
                continue
            name = item["name"]
            if name not in configs_map:
                config_order.append(name)
                configs_map[name] = []
            configs_map[name].append(item)

    for name in config_order:
        items = configs_map[name]
        if len(items) != expected_runs:
            print(f"Error: Benchmark configuration '{name}' appeared in {len(items)} passes, expected {expected_runs}.", file=sys.stderr)
            sys.exit(1)

    aggregated_results = []

    for name in config_order:
        items = configs_map[name]
        items_sorted = sorted(items, key=lambda x: float(x.get("avg_latency_ns", 0.0)))
        median_index = len(items_sorted) // 2
        selected_run_item = dict(items_sorted[median_index])
        aggregated_results.append(selected_run_item)

    os.makedirs(os.path.dirname(final_output_path), exist_ok=True)
    with open(final_output_path, "w", encoding="utf-8") as f:
        json.dump(aggregated_results, f, indent=2)
        f.write("\n")

    print(f"\nSuccessfully generated representative median aggregated benchmark results across {expected_runs} runs at: {final_output_path}")

def main():
    parser = argparse.ArgumentParser(description="Run SysCore Performance Benchmarks")
    parser.add_argument("--build-dir", default="build", help="Path to build directory (default: build)")
    parser.add_argument("--output", default="build/results.json", help="Path to output JSON file (default: build/results.json)")
    parser.add_argument("--runs", type=int, default=1, help="Number of benchmark passes to execute for median aggregation (default: 1)")
    args = parser.parse_args()

    build_dir = os.path.abspath(args.build_dir)
    output_path = os.path.abspath(args.output)
    runs = max(1, args.runs)

    if not os.path.exists(build_dir):
        print(f"Error: Build directory '{build_dir}' does not exist.", file=sys.stderr)
        sys.exit(1)

    if runs == 1:
        print(f"Executing {len(BENCHMARK_EXECUTABLES)} benchmark suites...")
        print(f"Machine-readable output destination: {output_path}\n")
        failed = run_single_pass(build_dir, output_path)
        if os.path.exists(output_path):
            print(f"\nSuccessfully generated benchmark results at: {output_path}")
        else:
            print(f"\nError: Output file '{output_path}' was not generated.", file=sys.stderr)
            sys.exit(1)
        sys.exit(failed)

    print(f"Executing {len(BENCHMARK_EXECUTABLES)} benchmark suites across {runs} passes for median aggregation...")
    print(f"Final output destination: {output_path}\n")

    pass_files = []
    total_failed = 0

    for r in range(1, runs + 1):
        print(f"================================================================================")
        print(f"Running benchmark suite: pass {r}/{runs}")
        print(f"================================================================================\n")
        pfile = os.path.join(build_dir, f"results_pass_{r}.json")
        failed = run_single_pass(build_dir, pfile)
        total_failed += failed
        pass_files.append(pfile)

    print(f"================================================================================")
    print(f"Aggregating results using median across {runs} runs...")
    print(f"================================================================================\n")
    aggregate_results(pass_files, output_path)

    for pfile in pass_files:
        if os.path.exists(pfile):
            try:
                os.remove(pfile)
            except OSError:
                pass

    sys.exit(0 if total_failed == 0 else 1)

if __name__ == "__main__":
    main()
