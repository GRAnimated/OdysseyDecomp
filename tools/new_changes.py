#!/usr/bin/env python3

import re
import subprocess
import sys

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

# Compute weighted byte size from changes in file_list.yml.
# The diff shows status lines changing; size is visible in context lines.
# We track the last seen `size:` value from context to pair with each status change.
NON_MATCHING = {"NonMatchingMinor", "NonMatchingMajor"}

file_list_diff = run(f"git diff {BASE} -- data/file_list.yml")

matching_bytes = 0
nonmatching_bytes = 0
last_size = None

for line in file_list_diff.splitlines():
    # Skip diff headers
    if line.startswith("+++") or line.startswith("---") or line.startswith("@@"):
        continue

    content = line[1:].strip() if len(line) > 1 else ""

    # Track size from context or added lines
    m = re.match(r"size:\s*(\d+)", content)
    if m:
        last_size = int(m.group(1))
        continue

    # Only process added status lines
    if not line.startswith("+"):
        continue

    m = re.match(r"status:\s*(\S+)", content)
    if m and last_size is not None:
        new_status = m.group(1)
        if new_status == "Matching":
            matching_bytes += last_size
        elif new_status in NON_MATCHING:
            nonmatching_bytes += last_size
        last_size = None

print("Status counts:")
for key, val in counts.items():
    print(f"  {key}: {val}")
sum_lines = 0
for name, lines in sorted(files.items()):
    sum_lines += lines
print(f"\nTotal added lines: {sum_lines}")
print(f"Matching added bytes: {matching_bytes / 1024:.1f} KB")
print(f"Non-Matching added bytes: {nonmatching_bytes / 1024:.1f} KB")
print(f"Total added bytes: {(matching_bytes + nonmatching_bytes) / 1024:.1f} KB")

if "--files" in sys.argv:
    print("\nModified files:")
    for name in sorted(files.keys()):
        stripped_name = name
        if stripped_name.startswith("lib/al/"):
            stripped_name = stripped_name[len("lib/al/"):]
        elif stripped_name.startswith("src/"):
            stripped_name = stripped_name[len("src/"):]
        print(stripped_name)
