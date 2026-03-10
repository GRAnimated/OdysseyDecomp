#!/usr/bin/env python3
"""
create_pr_branches.py — Create and push all PR branches from claude-testing-cleanup.

Branch structure:
  claude/wave-N        — accumulates all files for wave N, stacked on claude/wave-(N-1)
  claude/<PR-name>     — contains only that PR's files, branched from claude/wave-N

Each PR branch targets claude/wave-N as its base, so GitHub shows only the PR's diff.
Build/check and file_list.yml updates are handled separately by the Claude assistant.

Usage:
  python3 tools/create_pr_branches.py [--dry-run] [--start-wave N] [--only-wave N]

Options:
  --dry-run       Print commands without executing
  --start-wave N  Skip waves before N (resume after partial run)
  --only-wave N   Process only wave N
"""

import argparse
import json
import subprocess
import sys
import os
from pathlib import Path

REPO_ROOT = Path(__file__).parent.parent.resolve()
SOURCE_BRANCH = "claude-testing-cleanup"
REMOTE = "origin"
WAVE_BRANCH_PREFIX = "claude/wave-"
PR_BRANCH_PREFIX = "claude/"

# Infra PRs — handled separately (manual, not auto-branched)
INFRA_KEYS = {"infra-docs", "infra-al", "partial"}


def run(cmd, dry_run=False, check=True, capture=False):
    """Run a shell command."""
    print(f"  $ {' '.join(cmd)}")
    if dry_run:
        return subprocess.CompletedProcess(cmd, 0, stdout="", stderr="")
    kwargs = {"cwd": REPO_ROOT, "check": check}
    if capture:
        kwargs["capture_output"] = True
        kwargs["text"] = True
    return subprocess.run(cmd, **kwargs)


def branch_exists_remote(branch, dry_run=False):
    """Check if a branch exists on the remote."""
    result = subprocess.run(
        ["git", "ls-remote", "--heads", REMOTE, branch],
        cwd=REPO_ROOT, capture_output=True, text=True
    )
    return bool(result.stdout.strip())


def branch_exists_local(branch):
    """Check if a branch exists locally."""
    result = subprocess.run(
        ["git", "branch", "--list", branch],
        cwd=REPO_ROOT, capture_output=True, text=True
    )
    return bool(result.stdout.strip())


def pr_branch_name(pr_key):
    """Convert a PR key like 'src/Boss/BossRaid/BossRaidChain' to 'claude/BossRaidChain'."""
    name = pr_key.split("/")[-1]
    return f"{PR_BRANCH_PREFIX}{name}"


def wave_branch_name(wave_num):
    return f"{WAVE_BRANCH_PREFIX}{wave_num}"


def get_current_branch():
    result = subprocess.run(
        ["git", "rev-parse", "--abbrev-ref", "HEAD"],
        cwd=REPO_ROOT, capture_output=True, text=True, check=True
    )
    return result.stdout.strip()


def checkout(branch, dry_run=False):
    run(["git", "checkout", branch], dry_run=dry_run)


def create_branch(branch, base, dry_run=False):
    """Create a new local branch from base, deleting if already exists."""
    if branch_exists_local(branch):
        print(f"  [skip] local branch {branch} already exists, deleting and recreating")
        if not dry_run:
            subprocess.run(["git", "branch", "-D", branch], cwd=REPO_ROOT, check=True)
    run(["git", "checkout", "-b", branch, base], dry_run=dry_run)


def get_deleted_files(source_branch):
    """Return set of files deleted in source_branch relative to master."""
    result = subprocess.run(
        ["git", "diff", "--name-only", "--diff-filter=D", f"master..{source_branch}"],
        cwd=REPO_ROOT, capture_output=True, text=True, check=True
    )
    return set(result.stdout.splitlines())


def checkout_files_from(source_branch, files, deleted_files, dry_run=False):
    """Checkout specific files from source_branch; use git rm for deleted files."""
    to_checkout = [f for f in files if f not in deleted_files]
    to_delete = [f for f in files if f in deleted_files]
    if to_checkout:
        run(["git", "checkout", source_branch, "--"] + to_checkout, dry_run=dry_run)
    if to_delete:
        print(f"  [delete] Removing deleted files: {to_delete}")
        run(["git", "rm", "--ignore-unmatch", "-f"] + to_delete, dry_run=dry_run)


def commit(message, dry_run=False):
    """Stage all changes and commit."""
    run(["git", "add", "-A"], dry_run=dry_run)
    result = subprocess.run(
        ["git", "diff", "--cached", "--quiet"],
        cwd=REPO_ROOT
    )
    if result.returncode == 0:
        print("  [skip] nothing to commit")
        return False
    run(["git", "commit", "-m", message], dry_run=dry_run)
    return True


def push_branch(branch, dry_run=False):
    """Force-push branch to remote."""
    run(["git", "push", "--force", REMOTE, f"{branch}:{branch}"], dry_run=dry_run)



def main():
    parser = argparse.ArgumentParser(description="Create PR branches from claude-testing-cleanup")
    parser.add_argument("--dry-run", action="store_true", help="Print commands without executing")
    parser.add_argument("--start-wave", type=int, default=1, help="Skip waves before N")
    parser.add_argument("--only-wave", type=int, default=None, help="Process only wave N")
    args = parser.parse_args()

    dry_run = args.dry_run

    # Load PR plan
    plan_path = REPO_ROOT / "pr_dependency_tree.json"
    if not plan_path.exists():
        print("ERROR: pr_dependency_tree.json not found. Run tools/pr_dependency_tree.py first.")
        sys.exit(1)

    with open(plan_path) as f:
        data = json.load(f)

    waves = data["waves"]

    # Safety check: ensure we're not going to push to upstream
    remote_url_result = subprocess.run(
        ["git", "remote", "get-url", REMOTE],
        cwd=REPO_ROOT, capture_output=True, text=True, check=True
    )
    remote_url = remote_url_result.stdout.strip()
    if "MonsterDruide1" in remote_url:
        print(f"ERROR: Remote '{REMOTE}' points to upstream ({remote_url}). Refusing to push.")
        sys.exit(1)
    print(f"Remote: {remote_url}")

    original_branch = get_current_branch()
    print(f"Starting on branch: {original_branch}")
    print(f"Source branch: {SOURCE_BRANCH}")
    print()

    # Master SHA is the base for wave-1
    master_sha_result = subprocess.run(
        ["git", "rev-parse", "master"],
        cwd=REPO_ROOT, capture_output=True, text=True, check=True
    )
    master_sha = master_sha_result.stdout.strip()
    print(f"master SHA: {master_sha}")
    print()

    deleted_files = get_deleted_files(SOURCE_BRANCH)
    if deleted_files:
        print(f"Deleted files in {SOURCE_BRANCH}: {sorted(deleted_files)}")
    print()

    prev_wave_branch = "master"

    for wave_data in waves:
        wave_num = wave_data["wave"]
        prs = wave_data["prs"]

        if args.only_wave is not None and wave_num != args.only_wave:
            # Still need to advance prev_wave_branch
            prev_wave_branch = wave_branch_name(wave_num)
            continue

        if wave_num < args.start_wave:
            prev_wave_branch = wave_branch_name(wave_num)
            continue

        wave_branch = wave_branch_name(wave_num)
        print(f"{'='*60}")
        print(f"WAVE {wave_num} — {len(prs)} PRs")
        print(f"  Wave branch: {wave_branch}  (base: {prev_wave_branch})")
        print(f"{'='*60}")

        # ── Create wave branch ──────────────────────────────────────────
        print(f"\n[wave-{wave_num}] Creating wave branch...")
        create_branch(wave_branch, prev_wave_branch, dry_run=dry_run)

        # Collect all files for this wave
        all_wave_files = []
        for pr in prs:
            all_wave_files.extend(pr["files"])

        # Checkout all wave files from source branch
        print(f"[wave-{wave_num}] Checking out {len(all_wave_files)} files from {SOURCE_BRANCH}...")
        checkout_files_from(SOURCE_BRANCH, all_wave_files, deleted_files, dry_run=dry_run)
        commit(f"wave-{wave_num}: all files for wave {wave_num}", dry_run=dry_run)

        # Push wave branch
        print(f"[wave-{wave_num}] Pushing wave branch...")
        push_branch(wave_branch, dry_run=dry_run)

        # ── Create individual PR branches ───────────────────────────────
        for pr in prs:
            pr_key = pr["key"]
            pr_name = pr["name"]
            branch = pr_branch_name(pr_key)
            pr_files = pr["files"]

            print(f"\n  [{pr_name}] Creating PR branch: {branch}")
            print(f"  [{pr_name}] Files: {pr_files}")

            # Branch from the PREVIOUS wave (same base as the wave branch),
            # so the diff against claude/wave-N shows only this PR's files.
            create_branch(branch, prev_wave_branch, dry_run=dry_run)

            # Checkout only this PR's files from source branch
            checkout_files_from(SOURCE_BRANCH, pr_files, deleted_files, dry_run=dry_run)
            commit(f"feat: implement {pr_name}", dry_run=dry_run)

            # Push PR branch
            print(f"  [{pr_name}] Pushing PR branch...")
            push_branch(branch, dry_run=dry_run)

            # Return to wave branch for next PR
            checkout(wave_branch, dry_run=dry_run)

        prev_wave_branch = wave_branch
        print()

    # Return to original branch
    print(f"\nReturning to {original_branch}...")
    checkout(original_branch, dry_run=dry_run)

    print("\nDone!")
    print(f"Pushed wave branches: claude/wave-1 through claude/wave-{waves[-1]['wave']}")
    print(f"Pushed PR branches: {sum(len(w['prs']) for w in waves)} individual branches")
    print(f"\nAll PR branches target their wave branch. Create PRs at:")
    print(f"  https://github.com/GRAnimated/OdysseyDecomp/branches")


if __name__ == "__main__":
    main()
