#!/usr/bin/env python3

import subprocess

def run(cmd):
    return subprocess.check_output(cmd, shell=True, text=True)

BASE = "0dc7bc63a9a2fb5c158d58c59b33dfc7c3032207"
diff = run(f"git diff {BASE}")
for f in run(f"git diff --name-only --diff-filter=A {BASE}").splitlines():
    if not run(f"git ls-files '{f}'").strip():
        diff += run(f"diff -u /dev/null '{f}' || true")

counts = {"Matching": 0, "NonMatchingMinor": 0, "NonMatchingMajor": 0}
files = {}
current_file = None

for line in diff.splitlines():
    if line.startswith("+++ "):
        current_file = line[6:] if line.startswith("+++ b/") else line[4:]
        if current_file not in files:
            files[current_file] = 0
        continue
    if not line.startswith("+") or current_file is None:
        continue

    files[current_file] += 1

    for key in ("NonMatchingMinor", "NonMatchingMajor", "Matching"):
        if key in line:
            counts[key] += 1
            break

print("Status counts:")
for key, val in counts.items():
    print(f"  {key}: {val}")
sum_lines = 0
for name, lines in sorted(files.items()):
    sum_lines += lines
print(f"Total added lines: {sum_lines}")