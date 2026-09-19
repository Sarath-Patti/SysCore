#!/usr/bin/env python3
"""
SysCore Performance Regression Comparison Script
Compares machine-readable benchmark results against a stored baseline to detect performance regressions.
"""

import argparse
import json
import os
import shutil
import sys

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

def load_json_results(filepath):
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            data = json.load(f)
            if isinstance(data, list):
                # Convert list to dict keyed by benchmark name
                res_dict = {}
                for item in data:
                    if isinstance(item, dict) and "name" in item:
                        res_dict[item["name"]] = item
                return res_dict
            elif isinstance(data, dict):
                return data
            else:
                raise ValueError("JSON content is neither a list nor a dictionary.")
    except Exception as e:
        print(f"Error loading JSON file '{filepath}': {e}", file=sys.stderr)
        sys.exit(1)

def resolve_baseline_path(requested_path):
    if requested_path in ("auto", "benchmarks/baselines/baseline.json") or not os.path.exists(requested_path):
        platform_name = "linux" if sys.platform.startswith("linux") else ("macos" if sys.platform == "darwin" else "baseline")
        platform_baseline = f"benchmarks/baselines/{platform_name}.json"
        if os.path.exists(platform_baseline):
            return os.path.abspath(platform_baseline)
    return os.path.abspath(requested_path)

def main():
    parser = argparse.ArgumentParser(description="Compare SysCore Benchmark Results against Baseline")
    parser.add_argument("--current", default="build/results.json", help="Path to current benchmark results JSON (default: build/results.json)")
    parser.add_argument("--baseline", default="auto", help="Path to baseline benchmark JSON or 'auto' for platform detection (default: auto)")
    parser.add_argument("--threshold", type=float, default=25.0, help="Regression threshold percentage (default: 25.0)")
    parser.add_argument("--update-baseline", action="store_true", help="Update stored baseline file with current results")

    args = parser.parse_args()

    current_path = os.path.abspath(args.current)
    baseline_path = resolve_baseline_path(args.baseline)

    if args.update_baseline:
        if not os.path.exists(current_path):
            print(f"Error: Current results file '{current_path}' does not exist.", file=sys.stderr)
            sys.exit(1)
        os.makedirs(os.path.dirname(baseline_path), exist_ok=True)
        shutil.copyfile(current_path, baseline_path)
        print(f"Baseline successfully updated at: {baseline_path}")
        sys.exit(0)

    if not os.path.exists(baseline_path):
        print(f"Error: Baseline file '{baseline_path}' does not exist.", file=sys.stderr)
        print("Run with --update-baseline to initialize the baseline file.", file=sys.stderr)
        sys.exit(1)

    if not os.path.exists(current_path):
        print(f"Error: Current results file '{current_path}' does not exist.", file=sys.stderr)
        sys.exit(1)

    baseline_data = load_json_results(baseline_path)
    current_data = load_json_results(current_path)

    threshold = args.threshold
    pass_count = 0
    fail_count = 0
    skip_count = 0
    eval_count = 0

    print("=" * 100)
    print("SysCore Performance Regression Analysis")
    print("=" * 100)
    print(f"Baseline File : {baseline_path}")
    print(f"Current File  : {current_path}")
    print(f"Threshold     : {threshold:.1f}%\n")
    print(f"{'Benchmark Name':<45} {'Metric':<15} {'Baseline':<12} {'Current':<12} {'Change':<10} {'Status':<6}")
    print("-" * 100)

    for name, base_item in baseline_data.items():
        if name not in current_data:
            print(f"{name:<45} {'All':<15} {'N/A':<12} {'Missing':<12} {'N/A':<10} {'SKIP':<6}")
            skip_count += 1
            continue

        eval_count += 1
        curr_item = current_data[name]

        # 1. Latency Check
        b_lat = float(base_item.get("avg_latency_ns", 0.0))
        c_lat = float(curr_item.get("avg_latency_ns", 0.0))
        lat_diff = c_lat - b_lat
        lat_pct = (lat_diff / b_lat * 100.0) if b_lat > 0 else 0.0

        if lat_pct > threshold:
            lat_status = "FAIL"
            fail_count += 1
        else:
            lat_status = "PASS"
            pass_count += 1

        lat_change_str = f"+{lat_pct:.2f}%" if lat_pct >= 0 else f"{lat_pct:.2f}%"
        print(f"{name:<45} {'Latency (avg)':<15} {format_latency(b_lat):<12} {format_latency(c_lat):<12} {lat_change_str:<10} {lat_status:<6}")

        # 2. Throughput Check
        b_tp = float(base_item.get("throughput_ops_sec", 0.0))
        c_tp = float(curr_item.get("throughput_ops_sec", 0.0))

        if b_tp > 0 and c_tp > 0:
            tp_diff = b_tp - c_tp
            tp_pct = (tp_diff / b_tp * 100.0)  # drop percentage
            if tp_pct > threshold:
                tp_status = "FAIL"
                fail_count += 1
            else:
                tp_status = "PASS"
                pass_count += 1

            tp_change_pct = ((c_tp - b_tp) / b_tp * 100.0)
            tp_change_str = f"+{tp_change_pct:.2f}%" if tp_change_pct >= 0 else f"{tp_change_pct:.2f}%"
            print(f"{'':<45} {'Throughput':<15} {format_throughput(b_tp):<12} {format_throughput(c_tp):<12} {tp_change_str:<10} {tp_status:<6}")

    print("-" * 100)
    print(f"Summary: {eval_count} benchmark configurations evaluated. Total checks: {pass_count + fail_count + skip_count} ({pass_count} PASS, {skip_count} SKIP, {fail_count} FAIL).")
    print(f"Overall Status: {'PASS' if fail_count == 0 else 'FAIL'}")
    print("=" * 100 + "\n")

    sys.exit(0 if fail_count == 0 else 1)

if __name__ == "__main__":
    main()
