#!/usr/bin/env python3
import subprocess, os

os.chdir('/home/goose/decomp/OdysseyDecomp')

# Step 1: Check log
r = subprocess.run(['git', 'log', '--oneline', '-3'], capture_output=True, text=True, timeout=30)
with open('/home/goose/decomp/OdysseyDecomp/_git_output.txt', 'w') as f:
    f.write("=== git log --oneline -3 ===\n")
    f.write(r.stdout)
    f.write(r.stderr)
    f.write(f"\nRC: {r.returncode}\n\n")

# Check if commit already exists
if "Implement RunAwayNpc.o, Shibaken.o, ShellActorStateReaction.o" in r.stdout:
    with open('/home/goose/decomp/OdysseyDecomp/_git_output.txt', 'a') as f:
        f.write("COMMIT ALREADY EXISTS - skipping\n")
else:
    # Step 2: git add -A
    r2 = subprocess.run(['git', 'add', '-A'], capture_output=True, text=True, timeout=30)
    with open('/home/goose/decomp/OdysseyDecomp/_git_output.txt', 'a') as f:
        f.write("=== git add -A ===\n")
        f.write(r2.stdout)
        f.write(r2.stderr)
        f.write(f"\nRC: {r2.returncode}\n\n")

    # Step 3: git commit
    msg = """Implement RunAwayNpc.o, Shibaken.o, ShellActorStateReaction.o

RunAwayNpc: 34/37 matching (ctor C1/C2 93%, attackSensor 82%)
Shibaken: full implementation
ShellActorStateReaction: full implementation

Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"""

    r3 = subprocess.run(['git', 'commit', '-m', msg], capture_output=True, text=True, timeout=30)
    with open('/home/goose/decomp/OdysseyDecomp/_git_output.txt', 'a') as f:
        f.write("=== git commit ===\n")
        f.write(r3.stdout)
        f.write(r3.stderr)
        f.write(f"\nRC: {r3.returncode}\n\n")

# Step 4: Verify
r4 = subprocess.run(['git', 'log', '-1'], capture_output=True, text=True, timeout=30, env={**os.environ, 'GIT_PAGER': 'cat'})
with open('/home/goose/decomp/OdysseyDecomp/_git_output.txt', 'a') as f:
    f.write("=== git log -1 ===\n")
    f.write(r4.stdout)
    f.write(r4.stderr)
    f.write(f"\nRC: {r4.returncode}\n\n")

r5 = subprocess.run(['git', 'status'], capture_output=True, text=True, timeout=30)
with open('/home/goose/decomp/OdysseyDecomp/_git_output.txt', 'a') as f:
    f.write("=== git status ===\n")
    f.write(r5.stdout)
    f.write(r5.stderr)
    f.write(f"\nRC: {r5.returncode}\n\n")
    f.write("=== ALL DONE ===\n")
