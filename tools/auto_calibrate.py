"""Automated IMU calibration of a cube that is held in its zero position.

Runs the deployed app on the BBB with --calibrate (no torque, ends after 10 s),
records the raw IMU data on the host, computes the calibration values with
calib.py, writes them into config/cube_calibration.json and copies the config
back to the BBB.

Requires the SSH tunnel (connect_cube.sh) and an app that supports --calibrate
already deployed on the BBB.
"""
import argparse
import json
import subprocess
import sys
import time
from pathlib import Path

import pandas as pd

import calib

ROOT = Path(__file__).resolve().parent.parent
MANAGE = str(ROOT / "manage_cube.sh")
VIEWER = str(ROOT / "gui" / "cube_data_viewer.py")
CONFIG = ROOT / "config" / "cube_calibration.json"
DATA_DIR = ROOT / "ExperimentsData"

SSH_PORT = "48000"
SSH_TARGET = "debian@localhost"
RUN_TIMEOUT_S = 90

# Sanity limits on the raw-count data (cube must be still and in zero position)
MIN_SAMPLES = 300
MAX_ACCEL_STD = 50.0
MAX_GYRO_STD = 10.0
MAX_ACCEL_OFFSET = 2000.0
MAX_GYRO_OFFSET = 500.0

PAIRS_PER_LINE = 2


def detect_cube_ip():
    out = subprocess.run(["ssh", "-p", SSH_PORT, SSH_TARGET, "hostname -I"],
                         capture_output=True, text=True, timeout=20)
    if out.returncode != 0:
        raise RuntimeError(f"ssh hostname -I failed: {out.stderr.strip()}")
    return out.stdout.split()


def find_entry(config, cube, ips):
    for entry in config["cubes"]:
        if (cube is not None and entry["cube"] == cube) or (cube is None and entry["ip"] in ips):
            return entry
    if cube is not None:
        raise RuntimeError(f"cube {cube} not found in {CONFIG}")
    raise RuntimeError(f"none of the BBB addresses {ips} found in {CONFIG}; use --cube N")


def record_calibration_run(csv_path):
    """Starts the recorder and the app on the BBB and waits for both to finish."""
    subprocess.run([MANAGE, "-stopbbb"], check=True)
    recorder = subprocess.Popen([sys.executable, VIEWER, "--calib", "--out", str(csv_path)])
    app = subprocess.Popen([MANAGE, "-calbbb"])
    deadline = time.monotonic() + RUN_TIMEOUT_S
    try:
        for proc in (app, recorder):
            proc.wait(timeout=max(1.0, deadline - time.monotonic()))
    except subprocess.TimeoutExpired:
        raise RuntimeError("calibration run timed out")
    finally:
        for proc in (app, recorder):
            if proc.poll() is None:
                proc.kill()
    if app.returncode != 0:
        raise RuntimeError(f"app on BBB failed (exit code {app.returncode})")
    if recorder.returncode != 0:
        raise RuntimeError(f"recorder failed (exit code {recorder.returncode})")


def check_data(df, values):
    problems = []
    if len(df) < MIN_SAMPLES:
        problems.append(f"only {len(df)} samples (need >= {MIN_SAMPLES})")
    for prefix in ("S1", "S2"):
        for col, limit in ((f"{prefix}_mAx", MAX_ACCEL_STD), (f"{prefix}_mAy", MAX_ACCEL_STD),
                           (f"{prefix}_mWz", MAX_GYRO_STD)):
            std = df[col].std()
            if std > limit:
                problems.append(f"{col} std {std:.1f} > {limit} (cube moving?)")
    for key, value in values.items():
        if "Offset" not in key:
            continue
        limit = MAX_GYRO_OFFSET if "Gyro" in key else MAX_ACCEL_OFFSET
        if abs(value) > limit:
            problems.append(f"{key} = {value:.1f} exceeds +-{limit} (cube not in zero position?)")
    return problems


def fmt_value(v):
    if isinstance(v, float):
        return repr(round(v, 6))
    return json.dumps(v)


def dump_config(config):
    """Serializes like the hand-written file: calibration values two per line."""
    lines = ['{', '  "cubes": [']
    entries = []
    for entry in config["cubes"]:
        cal_items = [f'"{k}": {fmt_value(v)}' for k, v in entry["calibration"].items()]
        cal_lines = [", ".join(cal_items[i:i + PAIRS_PER_LINE])
                     for i in range(0, len(cal_items), PAIRS_PER_LINE)]
        entries.append("\n".join([
            '    {',
            f'      "ip": {json.dumps(entry["ip"])},',
            f'      "cube": {entry["cube"]},',
            '      "calibration": {',
            ",\n".join(f"        {line}" for line in cal_lines),
            '      }',
            '    }',
        ]))
    lines.append(",\n".join(entries))
    lines += ['  ]', '}']
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--cube", type=int, help="cube number (default: detect via IP of the BBB)")
    parser.add_argument("--csv", help="use this recorded csv instead of running the cube")
    parser.add_argument("--dry-run", action="store_true", help="print values, do not touch the config")
    args = parser.parse_args()

    original = CONFIG.read_text()
    config = json.loads(original)
    if dump_config(config) != original.rstrip("\n"):
        print("Warning: config layout differs from the serializer, file will be reformatted")

    entry = find_entry(config, args.cube, [] if args.cube is not None else detect_cube_ip())
    if args.csv:
        csv_path = Path(args.csv)
    else:
        print(f"Calibrating cube {entry['cube']} ({entry['ip']}), keep it in the zero position ...")
        csv_path = DATA_DIR / f"ImuCalib_{time.strftime('%Y%m%d_%H%M%S')}.csv"
        record_calibration_run(csv_path)

    df = pd.read_csv(csv_path)
    values = calib.compute_calibration(df)
    problems = check_data(df, values)
    if problems:
        print("Calibration rejected, config not modified:")
        for p in problems:
            print("  -", p)
        return 1

    print(f"Cube {entry['cube']} (data: {csv_path}, {len(df)} samples):")
    for key, new in values.items():
        print(f"  {key:18s} {entry['calibration'][key]:>12.6f} -> {new:>12.6f}")
    if args.dry_run:
        return 0

    entry["calibration"].update({k: round(v, 6) for k, v in values.items()})
    CONFIG.write_text(dump_config(config) + ("\n" if original.endswith("\n") else ""))
    print(f"Updated {CONFIG}")
    if args.csv:
        return 0
    subprocess.run([MANAGE, "-cfgbbb"], check=True)
    print("Config copied to BBB")
    return 0


if __name__ == "__main__":
    sys.exit(main())
