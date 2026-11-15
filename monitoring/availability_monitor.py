#!/usr/bin/env python3
"""Sample the Railway Reservation System's console startup availability."""
from __future__ import annotations
import argparse, csv, datetime as dt, subprocess, sys, time
from pathlib import Path
from typing import Any

def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="milliseconds")

def probe_application(app: Path, timeout_seconds: float, cwd: Path | None = None) -> dict[str, Any]:
    started_at = utc_now()
    start = time.monotonic()
    try:
        completed = subprocess.run([str(app)], input="8\n", text=True, encoding="utf-8",
            errors="replace", capture_output=True, timeout=timeout_seconds,
            cwd=str(cwd) if cwd else None, check=False)
        output = completed.stdout or ""
        elapsed_ms = round((time.monotonic() - start) * 1000, 2)
        menu_seen = "Enter choice:" in output
        exit_seen = "Exiting system. Goodbye!" in output
        ok = completed.returncode == 0 and menu_seen and exit_seen
        reason = "ok" if ok else (
            f"exit_code={completed.returncode}; menu_seen={menu_seen}; exit_seen={exit_seen}")
        if completed.stderr.strip():
            reason += f"; stderr={completed.stderr.strip()[:300]}"
        return {"started_at_utc": started_at, "finished_at_utc": utc_now(),
            "available": ok, "duration_ms": elapsed_ms,
            "exit_code": completed.returncode, "reason": reason}
    except subprocess.TimeoutExpired:
        return {"started_at_utc": started_at, "finished_at_utc": utc_now(),
            "available": False, "duration_ms": round((time.monotonic()-start)*1000, 2),
            "exit_code": "", "reason": f"timeout_after_{timeout_seconds}s"}
    except OSError as exc:
        return {"started_at_utc": started_at, "finished_at_utc": utc_now(),
            "available": False, "duration_ms": round((time.monotonic()-start)*1000, 2),
            "exit_code": "", "reason": f"launch_error: {exc}"}

def write_report(path: Path, rows: list[dict[str, Any]]) -> float:
    path.parent.mkdir(parents=True, exist_ok=True)
    fields = ["started_at_utc","finished_at_utc","available","duration_ms","exit_code","reason"]
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    passed = sum(1 for row in rows if row["available"])
    return passed / len(rows) * 100.0 if rows else 0.0

def positive_int(value: str) -> int:
    number = int(value)
    if number < 1: raise argparse.ArgumentTypeError("must be at least 1")
    return number

def non_negative_float(value: str) -> float:
    number = float(value)
    if number < 0: raise argparse.ArgumentTypeError("must be zero or greater")
    return number

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--app", required=True, type=Path, help="Path to built railway executable")
    parser.add_argument("--working-directory", type=Path, default=Path.cwd())
    parser.add_argument("--checks", type=positive_int, default=10)
    parser.add_argument("--interval-seconds", type=non_negative_float, default=60)
    parser.add_argument("--timeout-seconds", type=positive_int, default=10)
    parser.add_argument("--output", type=Path, default=Path("availability-evidence.csv"))
    args = parser.parse_args()
    app = args.app.expanduser().resolve()
    if not app.is_file(): parser.error(f"application executable does not exist: {app}")
    rows: list[dict[str, Any]] = []
    print(f"Monitoring {app} for {args.checks} startup probes.")
    print("Each probe opens the main menu, sends Exit, and records timestamps.")
    try:
        for index in range(args.checks):
            row = probe_application(app, args.timeout_seconds, args.working_directory.resolve())
            rows.append(row)
            state = "AVAILABLE" if row["available"] else "UNAVAILABLE"
            print(f"[{index+1}/{args.checks}] {state} | {row['duration_ms']} ms | {row['reason']}")
            if index + 1 < args.checks: time.sleep(args.interval_seconds)
    except KeyboardInterrupt:
        print("\nInterrupted; writing collected probes.")
    percentage = write_report(args.output, rows)
    passed = sum(1 for row in rows if row["available"])
    print(f"\nProbe availability: {passed}/{len(rows)} = {percentage:.2f}%")
    print(f"Evidence CSV: {args.output.resolve()}")
    if not rows: return 2
    return 0 if passed == len(rows) else 1

if __name__ == "__main__":
    sys.exit(main())
