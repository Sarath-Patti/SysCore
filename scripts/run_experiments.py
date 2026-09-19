#!/usr/bin/env python3
"""
SysCore Phase 4 Performance Analysis & Comparative Experiments Orchestrator

Executes IPC, Synchronization, Concurrency Scaling, Resource Measurement,
and Baseline-vs-Optimized experiments. Generates structured JSON and flattened CSV results.
"""

import csv
import json
import os
import re
import subprocess
import sys

def parse_benchmark_metadata(name):
    """
    Extracts mechanism, workload, payload size, and concurrency level from benchmark name.
    """
    mechanism = "Unknown"
    workload = "General"
    payload_size = None
    concurrency = 1

    # Extract Payload Size if present
    payload_match = re.search(r"Payload\s*(\d+)B", name, re.IGNORECASE)
    if payload_match:
        payload_size = int(payload_match.group(1))

    # Extract Concurrency/Threads if present
    thread_match = re.search(r"(\d+)\s*Thread", name, re.IGNORECASE)
    if thread_match:
        concurrency = int(thread_match.group(1))

    # Identify Mechanism & Workload
    if "IPC Pipe" in name or "Pipe" in name:
        mechanism = "Pipe"
        workload = "IPC Payload Transfer"
    elif "IPC Shared Memory" in name or "Shared Memory" in name:
        mechanism = "Shared Memory"
        workload = "IPC Payload Transfer"
    elif "IPC Message Queue" in name or "Message Queue" in name:
        mechanism = "POSIX Message Queue"
        workload = "IPC Payload Transfer"
    elif "Synchronization Mutex" in name or "Mutex" in name:
        mechanism = "Mutex"
        workload = "Synchronization Contention"
    elif "Synchronization Semaphore" in name or "Semaphore" in name:
        mechanism = "Semaphore"
        workload = "Synchronization Contention"
    elif "Synchronization RWLock" in name or "RWLock" in name:
        mechanism = "RWLock"
        workload = "Synchronization Contention"
    elif "Synchronization CondVar" in name or "CondVar" in name:
        mechanism = "Condition Variable"
        workload = "Synchronization Signaling"
    elif "Baseline" in name:
        mechanism = "Baseline (Unamortized Locked)"
        workload = "Locked Payload Transfer"
        payload_size = 4096
    elif "Optimized" in name:
        mechanism = "Optimized (Block-Amortized Locked)"
        workload = "Locked Payload Transfer"
        payload_size = 4096

    return mechanism, workload, payload_size, concurrency

def run_experiment_binaries(build_dir, json_temp_file):
    """
    Executes Phase 4 experiment binaries with environment variable setting.
    """
    binaries = [
        "exp_ipc_comparison",
        "exp_sync_comparison",
        "exp_baseline_vs_optimized"
    ]

    if os.path.exists(json_temp_file):
        os.remove(json_temp_file)

    env = os.environ.copy()
    env["SYSCORE_BENCHMARK_JSON_FILE"] = json_temp_file

    for bin_name in binaries:
        bin_path = os.path.join(build_dir, bin_name)
        if not os.path.exists(bin_path):
            print(f"Error: Binary '{bin_path}' not found. Please build the project first.", file=sys.stderr)
            sys.exit(1)

        print(f"Executing experiment binary: {bin_name} ...")
        res = subprocess.run([bin_path], env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if res.returncode != 0:
            print(f"Error executing {bin_name}: {res.stderr}", file=sys.stderr)
            sys.exit(1)

def format_latency(ns):
    if ns < 1000.0:
        return f"{ns:.2f} ns"
    elif ns < 1000000.0:
        return f"{ns / 1000.0:.2f} us"
    elif ns < 1000000000.0:
        return f"{ns / 1000000.0:.2f} ms"
    else:
        return f"{ns / 1000000000.0:.2f} s"

def format_throughput(ops_sec):
    if ops_sec >= 1e9:
        return f"{ops_sec / 1e9:.2f}G ops/s"
    elif ops_sec >= 1e6:
        return f"{ops_sec / 1e6:.2f}M ops/s"
    elif ops_sec >= 1e3:
        return f"{ops_sec / 1e3:.2f}K ops/s"
    else:
        return f"{ops_sec:.2f} ops/s"

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    build_dir = os.path.join(project_root, "build")
    
    json_temp_file = os.path.join(build_dir, "raw_exp_results.json")
    json_out_file = os.path.join(build_dir, "experiments_results.json")
    csv_out_file = os.path.join(build_dir, "experiments_results.csv")

    run_experiment_binaries(build_dir, json_temp_file)

    if not os.path.exists(json_temp_file):
        print(f"Error: Raw experiment output file '{json_temp_file}' was not produced.", file=sys.stderr)
        sys.exit(1)

    with open(json_temp_file, "r", encoding="utf-8") as f:
        raw_results = json.load(f)

    # Process and enrich results with metadata and derived metrics
    enriched_results = []
    
    # 1. First pass: parse metadata
    for item in raw_results:
        name = item.get("name", "Unnamed")
        mechanism, workload, payload_size, concurrency = parse_benchmark_metadata(name)
        
        entry = dict(item)
        entry["mechanism"] = mechanism
        entry["workload"] = workload
        entry["payload_size_bytes"] = payload_size if payload_size is not None else "N/A"
        entry["concurrency"] = concurrency
        entry["scaling_efficiency"] = 1.0
        entry["relative_change_pct"] = 0.0
        enriched_results.append(entry)

    # 2. Second pass: compute Concurrency Scaling Efficiency
    # Group by (workload, mechanism)
    sync_groups = {}
    for entry in enriched_results:
        if entry["workload"] in ["Synchronization Contention", "Synchronization Signaling"]:
            key = (entry["workload"], entry["mechanism"])
            if key not in sync_groups:
                sync_groups[key] = {}
            sync_groups[key][entry["concurrency"]] = entry

    for key, conc_map in sync_groups.items():
        if 1 in conc_map:
            t1 = conc_map[1]["throughput_ops_sec"]
            for c, entry in conc_map.items():
                if c == 1:
                    entry["scaling_efficiency"] = 1.0
                elif t1 > 0:
                    # Scaling Efficiency E_N = T_N / (N * T_1)
                    tn = entry["throughput_ops_sec"]
                    eff = tn / (c * t1)
                    entry["scaling_efficiency"] = round(eff, 4)

    # 3. Third pass: compute Baseline vs Optimized relative change
    baseline_entry = None
    optimized_entry = None
    for entry in enriched_results:
        if "Baseline" in entry["name"]:
            baseline_entry = entry
        elif "Optimized" in entry["name"]:
            optimized_entry = entry

    if baseline_entry and optimized_entry:
        b_tp = baseline_entry["throughput_ops_sec"]
        o_tp = optimized_entry["throughput_ops_sec"]
        if b_tp > 0:
            rel_change = ((o_tp - b_tp) / b_tp) * 100.0
            optimized_entry["relative_change_pct"] = round(rel_change, 2)
            baseline_entry["relative_change_pct"] = 0.0

    # Write enriched JSON
    with open(json_out_file, "w", encoding="utf-8") as f:
        json.dump(enriched_results, f, indent=2)
    print(f"\nSaved machine-readable JSON results to: {json_out_file}")

    # Write flattened CSV
    csv_headers = [
        "mechanism", "workload", "payload_size_bytes", "concurrency",
        "avg_latency_ns", "p50_latency_ns", "p95_latency_ns", "p99_latency_ns",
        "throughput_ops_sec", "user_cpu_sec", "sys_cpu_sec", "total_cpu_sec",
        "relative_change_pct", "scaling_efficiency"
    ]

    with open(csv_out_file, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=csv_headers, extrasaction="ignore")
        writer.writeheader()
        for entry in enriched_results:
            writer.writerow(entry)
    print(f"Saved flattened CSV results to: {csv_out_file}\n")

    # Clean up temporary raw json
    if os.path.exists(json_temp_file):
        os.remove(json_temp_file)

    # Display Consolidated Summary Tables
    print("=" * 115)
    print("SYSCORE PHASE 4 COMPARATIVE PERFORMANCE ANALYSIS REPORT")
    print("=" * 115)

    # Table 1: IPC Comparison
    print("\n1. IPC COMPARISON SUMMARY TABLE")
    print("-" * 115)
    print(f"{'Mechanism':<22} {'Payload':<10} {'Avg Latency':<15} {'p50 Latency':<15} {'p95 Latency':<15} {'Throughput':<15} {'Total CPU':<12}")
    print("-" * 115)
    for entry in enriched_results:
        if entry["workload"] == "IPC Payload Transfer":
            p_str = f"{entry['payload_size_bytes']} B"
            print(f"{entry['mechanism']:<22} {p_str:<10} {format_latency(entry['avg_latency_ns']):<15} "
                  f"{format_latency(entry['p50_latency_ns']):<15} {format_latency(entry['p95_latency_ns']):<15} "
                  f"{format_throughput(entry['throughput_ops_sec']):<15} {entry['total_cpu_sec']:.6f} s")

    # Table 2: Synchronization & Concurrency Scaling
    print("\n2. SYNCHRONIZATION & CONCURRENCY SCALING SUMMARY TABLE")
    print("-" * 115)
    print(f"{'Mechanism':<22} {'Threads':<10} {'Avg Latency':<15} {'p50 Latency':<15} {'Throughput':<15} {'Scaling Eff.':<15} {'Total CPU':<12}")
    print("-" * 115)
    for entry in enriched_results:
        if entry["workload"] in ["Synchronization Contention", "Synchronization Signaling"]:
            t_str = f"{entry['concurrency']} T"
            eff_str = f"{entry['scaling_efficiency'] * 100:.1f}%"
            print(f"{entry['mechanism']:<22} {t_str:<10} {format_latency(entry['avg_latency_ns']):<15} "
                  f"{format_latency(entry['p50_latency_ns']):<15} {format_throughput(entry['throughput_ops_sec']):<15} "
                  f"{eff_str:<15} {entry['total_cpu_sec']:.6f} s")

    # Table 3: Baseline vs Optimized Optimization Experiment
    print("\n3. BASELINE VS OPTIMIZED EXPERIMENT SUMMARY TABLE")
    print("-" * 115)
    print(f"{'Implementation':<35} {'Payload':<10} {'Avg Latency':<15} {'Throughput':<15} {'Rel Change':<12} {'Total CPU':<12}")
    print("-" * 115)
    for entry in enriched_results:
        if entry["workload"] == "Locked Payload Transfer":
            p_str = f"{entry['payload_size_bytes']} B"
            rel_str = f"+{entry['relative_change_pct']:.2f}%" if entry['relative_change_pct'] > 0 else f"{entry['relative_change_pct']:.2f}%"
            print(f"{entry['mechanism']:<35} {p_str:<10} {format_latency(entry['avg_latency_ns']):<15} "
                  f"{format_throughput(entry['throughput_ops_sec']):<15} {rel_str:<12} {entry['total_cpu_sec']:.6f} s")
    print("=" * 115 + "\n")

if __name__ == "__main__":
    main()
