#!/usr/bin/env python3
import glob
import subprocess
import sys

EXAMPLES_GLOB = "examples/*.mylang"


def main():
    paths = sorted(glob.glob(EXAMPLES_GLOB))
    if not paths:
        print(f"No files found matching {EXAMPLES_GLOB}")
        return 1

    failures = []
    for path in paths:
        print(f"Checking {path}...", end=" ")
        try:
            result = subprocess.run(["./build/basic", path], capture_output=True, text=True)
        except FileNotFoundError:
            print("failed")
            print("Error: './build/basic' executable not found")
            return 1

        if result.returncode != 0:
            print("failed")
            print(f"Return code: {result.returncode}")
            print(result.stdout)
            print(result.stderr)
            failures.append(path)
        else:
            print("ok")

    if failures:
        print(f"\n{len(failures)} file(s) failed:")
        for path in failures:
            print(f" - {path}")
        return 1

    print("All example files ran successfully.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
