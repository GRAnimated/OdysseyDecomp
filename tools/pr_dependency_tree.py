#!/usr/bin/env python3
"""
Generate a PR dependency graph for the claude-testing-cleanup branch.

Key rules:
- One PR = one implementation (.cpp + .h) plus any header-only src files and
  al/ files that belong to it.
- Merge order is determined ONLY by header-to-header #include relationships.
  A .cpp including another class's .h is NOT a merge-order constraint — the .h
  can be merged independently first.
- Cycles (header-to-header mutual includes) are merged into one PR.
- al/ files are assigned after topo-sort to the earliest-wave PR that uses them.
- Output: text waves file + DOT graph (render with: dot -Tsvg pr_graph.dot -o pr_graph.svg)

Usage:
    python3 tools/pr_dependency_tree.py [--output pr_dependency_tree.txt] [--dot pr_graph.dot]
"""

import subprocess
import re
import sys
import os
import json
from pathlib import Path
from collections import defaultdict, deque

REPO_ROOT = Path(__file__).parent.parent

PARTIAL_CPP_FILES = {
    "src/Enemy/Pecho.cpp",
    "src/MapObj/TrampleSwitch.cpp",
    "src/Player/PlayerTrigger.cpp",
    "src/Scene/ProjectActorFactory.cpp",
    "src/Scene/SceneEventNotifier.cpp",
    "src/Scene/StageScene.cpp",
}

BRANCH_WIDE_PREFIXES = [
    "CLAUDE", "docs/", "flake.nix", "data/", "manual_review",
    "task_list", "tools/",
]

# Manual assignment: al file basename -> src stem that motivated the change.
# Used as a hint; actual assignment happens post-topo-sort to avoid cycles.
AL_FILE_MANUAL_ASSIGNMENT = {
    "CollisionParts.h":            "src/Npc/ShibakenFunction",
    "DemoActorHolder.h":           "src/Npc/OpeningStageStartCapManHero",
    "DemoFunction.h":              "src/Npc/OpeningStageStartCapManHero",
    "EventFlowActorParamHolder.h": "src/Npc/TalkNpcParam",
    "EventFlowChoiceInfo.h":       "src/Npc/NpcEventCtrlInfo",
    "EventFlowChoiceInfo.cpp":     "src/Npc/NpcEventCtrlInfo",
    "EventFlowEventData.h":        "src/Npc/HelpNpc",
    "EventFlowExecutor.h":         "src/Boss/BossRaid/BossRaidElectric",
    "EventFlowExecutorHolder.h":   "src/Npc/NpcEventCtrlInfo",
    "EventFlowExecutorHolder.cpp": "src/Npc/NpcEventCtrlInfo",
    "EventFlowMovement.h":         "src/Npc/EventActorMovementTurn",
    "EventFlowMovementRailBlend.h":"src/Npc/TalkNpc",
    "EventFlowUtil.h":             "src/Npc/OpeningStageStartCapManHero",
    "EventFlowWatchParam.h":       "src/Npc/TalkNpcParam",
    "EventFlowWatchParam.cpp":     "src/Npc/TalkNpcParam",
    "IEventFlowQueryJudge.h":      "src/Npc/FukuwaraiNpc",
    "JointAimInfo.h":              "src/Boss/BossRaid/BossRaidNpc",
    "JointAimInfo.cpp":            "src/Boss/BossRaid/BossRaidNpc",
    "JointControllerKeeper.h":     "src/Boss/BossRaid/BossRaidElectric",
    "JointLookAtController.h":     "src/Npc/NpcJointLookAtController",
    "JointLookAtInfo.h":           "src/Npc/TalkNpcParam",
    "JointLookAtParam.h":          "src/Npc/TalkNpcParam",
    "JointQuatController.h":       "src/Npc/FrogStateWaitJump",
    "JointSpringControllerHolder.h":"src/Npc/OpeningStageStartCapManHero",
    "BalloonOrderGroupHolder.h":   "src/Npc/NpcEventCtrlInfo",
    "LayoutActorGroup.h":          "src/Npc/NpcEventDirector",
    "LayoutActorGroup.cpp":        "src/Npc/NpcEventDirector",
    "ReplaceTagProcessor.h":       "src/Layout/ProjectReplaceTagProcessor",
    "ActorAnimFunction.h":         "src/Boss/BossUtil/BossUtil",
    "ActorModelFunction.h":        "src/Boss/BossRaid/BossRaidElectric",
    "CustomTagProcessor.h":        "src/Layout/ProjectReplaceTagProcessor",
    "ModelShapeUtil.h":            "src/Npc/SkeletonDynamics",
    "RumbleCalculator.h":          "src/Npc/NpcStateWait",
}

# ---------------------------------------------------------------------------
# Utilities
# ---------------------------------------------------------------------------

def get_changed_files():
    result = subprocess.run(
        ["git", "diff", "master..claude-testing-cleanup", "--name-only"],
        capture_output=True, text=True, cwd=REPO_ROOT
    )
    return [f.strip() for f in result.stdout.strip().splitlines() if f.strip()]


def parse_includes(filepath, headers_only=False):
    """Return basenames of local #include directives. If headers_only, skip .cpp files."""
    if headers_only and not str(filepath).endswith(".h"):
        return []
    includes = []
    try:
        with open(REPO_ROOT / filepath) as fh:
            for line in fh:
                m = re.match(r'\s*#\s*include\s+"([^"]+)"', line)
                if m:
                    includes.append(os.path.basename(m.group(1)))
    except FileNotFoundError:
        pass
    return includes


def file_stem(filepath):
    p = Path(filepath)
    return str(p.parent / p.stem)


# ---------------------------------------------------------------------------
# Step 1: Classify
# ---------------------------------------------------------------------------

def classify(changed_files):
    branch_wide, al_files, partial_cpp, src_files = [], [], [], []
    for f in changed_files:
        if any(f.startswith(p) or f == p for p in BRANCH_WIDE_PREFIXES):
            branch_wide.append(f)
        elif f.startswith("lib/al/"):
            al_files.append(f)
        elif f in PARTIAL_CPP_FILES:
            partial_cpp.append(f)
        elif f.startswith("src/"):
            src_files.append(f)
    return branch_wide, al_files, partial_cpp, src_files


# ---------------------------------------------------------------------------
# Step 2: Build class PR units
# ---------------------------------------------------------------------------

def build_class_prs(src_files):
    stem_to_files = defaultdict(list)
    for f in src_files:
        stem_to_files[file_stem(f)].append(f)
    prs = {}
    for s, files in stem_to_files.items():
        prs[s] = {
            "name": Path(s).name,
            "directory": str(Path(s).parent),
            "files": sorted(files),
            "has_cpp": any(f.endswith(".cpp") for f in files),
            "has_header": any(f.endswith(".h") for f in files),
        }
    return prs


# ---------------------------------------------------------------------------
# Step 3: Merge header-only PRs into their users
# ---------------------------------------------------------------------------

def merge_header_only_prs(class_prs):
    def build_header_map(prs):
        hm = {}
        for pk, info in prs.items():
            for f in info["files"]:
                if f.endswith(".h"):
                    hm[os.path.basename(f)] = pk
        return hm

    changed = True
    while changed:
        changed = False
        header_map = build_header_map(class_prs)
        header_only = [pk for pk, info in class_prs.items() if not info["has_cpp"]]
        if not header_only:
            break

        for ho_pk in header_only:
            ho_dir = class_prs[ho_pk]["directory"]
            ho_hdrs = {os.path.basename(f) for f in class_prs[ho_pk]["files"] if f.endswith(".h")}

            # Find all class PRs that include this header (from any of their files)
            users = set()
            for pk, info in class_prs.items():
                if pk == ho_pk:
                    continue
                for cf in info["files"]:
                    if any(inc in ho_hdrs for inc in parse_includes(cf)):
                        users.add(pk)

            if not users:
                # No users found in class PRs — try same-dir impl
                same_dir = [pk for pk in class_prs
                            if pk != ho_pk
                            and class_prs[pk]["directory"] == ho_dir
                            and class_prs[pk]["has_cpp"]]
                if same_dir:
                    users = {sorted(same_dir)[0]}
                else:
                    class_prs[ho_pk]["absorb_into_partial"] = True
                    continue

            # Pick best target: same-dir impl > any impl > first alphabetically
            same_dir_impl = [pk for pk in users
                             if class_prs[pk]["directory"] == ho_dir and class_prs[pk]["has_cpp"]]
            any_impl = [pk for pk in users if class_prs[pk]["has_cpp"]]
            target = sorted(same_dir_impl or any_impl or sorted(users))[0]

            class_prs[target]["files"].extend(class_prs[ho_pk]["files"])
            class_prs[target]["files"] = sorted(set(class_prs[target]["files"]))
            class_prs[target]["has_header"] = True
            del class_prs[ho_pk]
            changed = True
            break

    orphaned = []
    for pk in list(class_prs.keys()):
        if class_prs[pk].get("absorb_into_partial"):
            orphaned.extend(class_prs[pk]["files"])
            del class_prs[pk]

    return class_prs, orphaned


# ---------------------------------------------------------------------------
# Step 4: Build dependency graph — HEADERS ONLY
#
# Only .h -> .h includes create merge-order constraints.
# A .cpp including another class's .h does NOT — the header can be merged first.
# ---------------------------------------------------------------------------

def build_dep_graph(all_prs, headers_only=True):
    """
    headers_only=True: only header files' includes create dependency edges.
    This prevents false cycles from .cpp files including sibling headers.
    """
    header_map = {}  # basename -> pr_key
    for pk, info in all_prs.items():
        for f in info["files"]:
            if f.endswith(".h"):
                header_map[os.path.basename(f)] = pk

    deps = {pk: set() for pk in all_prs}
    for pk, info in all_prs.items():
        for f in info["files"]:
            # Only follow includes from .h files for ordering
            if headers_only and not f.endswith(".h"):
                continue
            for inc in parse_includes(f):
                if inc in header_map and header_map[inc] != pk:
                    deps[pk].add(header_map[inc])

    return deps


# ---------------------------------------------------------------------------
# Step 5: SCC (Tarjan's) — now only fires on true header-to-header cycles
# ---------------------------------------------------------------------------

def tarjan_scc(nodes, adj):
    index_counter = [0]
    stack, lowlinks, index, on_stack, sccs = [], {}, {}, {}, []

    def strongconnect(v):
        index[v] = lowlinks[v] = index_counter[0]
        index_counter[0] += 1
        stack.append(v)
        on_stack[v] = True
        for w in adj.get(v, set()):
            if w not in index:
                strongconnect(w)
                lowlinks[v] = min(lowlinks[v], lowlinks[w])
            elif on_stack.get(w):
                lowlinks[v] = min(lowlinks[v], index[w])
        if lowlinks[v] == index[v]:
            scc = set()
            while True:
                w = stack.pop()
                on_stack[w] = False
                scc.add(w)
                if w == v:
                    break
            sccs.append(frozenset(scc))

    for v in nodes:
        if v not in index:
            strongconnect(v)
    return sccs


def merge_cycles(all_prs, deps):
    sccs = tarjan_scc(list(all_prs.keys()), deps)
    key_to_merged, merged = {}, {}

    for scc in sccs:
        if len(scc) == 1:
            k = next(iter(scc))
            key_to_merged[k] = k
            merged[k] = all_prs[k]
        else:
            sorted_keys = sorted(scc)
            mk = "+".join(sorted_keys)
            all_files = sorted(set(f for k in sorted_keys for f in all_prs[k]["files"]))
            dirs = [all_prs[k]["directory"] for k in sorted_keys]
            merged[mk] = {
                "name": " + ".join(all_prs[k]["name"] for k in sorted_keys),
                "directory": dirs[0],
                "files": all_files,
                "has_cpp": any(f.endswith(".cpp") for f in all_files),
                "has_header": any(f.endswith(".h") for f in all_files),
                "merged_from": sorted_keys,
            }
            for k in sorted_keys:
                key_to_merged[k] = mk

    merged_deps = {k: set() for k in merged}
    for old_k, dep_set in deps.items():
        nk = key_to_merged[old_k]
        for dep in dep_set:
            nd = key_to_merged[dep]
            if nd != nk:
                merged_deps[nk].add(nd)

    return merged, merged_deps


# ---------------------------------------------------------------------------
# Step 5b: Break .cpp-only back-edges to make graph a DAG
# ---------------------------------------------------------------------------

def break_cycles_greedily(all_prs, deps):
    """
    Remove back-edges (identified by DFS) until the graph is acyclic.
    These back-edges come from .cpp files including sibling .h files — they
    represent compile-order preferences, not hard merge-order requirements.
    """
    deps = {k: set(v) for k, v in deps.items()}  # deep copy

    visited, on_stack = set(), set()
    to_remove = []

    def dfs(v):
        visited.add(v)
        on_stack.add(v)
        for w in sorted(deps.get(v, [])):
            if w not in visited:
                dfs(w)
            elif w in on_stack:
                to_remove.append((v, w))
        on_stack.discard(v)

    import sys
    sys.setrecursionlimit(10000)
    for v in sorted(all_prs):
        if v not in visited:
            dfs(v)

    for a, b in to_remove:
        deps[a].discard(b)

    return deps


# ---------------------------------------------------------------------------
# Step 6: Topo sort -> waves
# ---------------------------------------------------------------------------

def compute_waves(all_prs, deps):
    in_degree = {k: 0 for k in all_prs}
    rev = defaultdict(set)
    for pk, dep_set in deps.items():
        for dep in dep_set:
            in_degree[pk] += 1
            rev[dep].add(pk)

    ready = sorted(k for k, d in in_degree.items() if d == 0)
    waves = []
    while ready:
        waves.append(sorted(ready))
        nxt = []
        for node in ready:
            for dep in sorted(rev[node]):
                in_degree[dep] -= 1
                if in_degree[dep] == 0:
                    nxt.append(dep)
        ready = nxt

    if sum(len(w) for w in waves) != len(all_prs):
        remaining = sorted(set(all_prs) - {k for w in waves for k in w})
        print(f"WARNING: {len(remaining)} nodes unplaced", file=sys.stderr)
        waves.append(remaining)

    return waves


# ---------------------------------------------------------------------------
# Step 7: Assign al/ files post-topo-sort (no cycles possible)
# ---------------------------------------------------------------------------

def assign_al_files(al_files, all_prs, waves):
    pr_wave = {pk: i for i, wave in enumerate(waves) for pk in wave}

    al_header_map = {os.path.basename(f): f for f in al_files if f.endswith(".h")}
    al_cpp_map = {file_stem(f): f for f in al_files if f.endswith(".cpp")}

    # For each al header, find which PRs include it (from any file) + wave position
    al_users = defaultdict(list)
    for pk, info in all_prs.items():
        for cf in info["files"]:
            for inc in parse_includes(cf):
                if inc in al_header_map:
                    al_users[inc].append((pr_wave.get(pk, 9999), pk))

    misc_al, assigned = [], set()

    for f in al_files:
        base = os.path.basename(f)
        if f.endswith(".cpp"):
            continue  # handled via header pairing

        users = sorted(al_users.get(base, []))
        hint = AL_FILE_MANUAL_ASSIGNMENT.get(base)

        # Prefer hint if valid; else earliest-wave user
        target = None
        if hint and hint in all_prs:
            target = hint
        elif users:
            target = users[0][1]

        if target:
            all_prs[target]["files"].append(f)
            all_prs[target]["files"] = sorted(set(all_prs[target]["files"]))
            all_prs[target]["has_header"] = True
            assigned.add(f)
            # Pair with cpp if present
            al_s = file_stem(f)
            if al_s in al_cpp_map:
                cpp_f = al_cpp_map[al_s]
                all_prs[target]["files"].append(cpp_f)
                all_prs[target]["files"] = sorted(set(all_prs[target]["files"]))
                all_prs[target]["has_cpp"] = True
                assigned.add(cpp_f)
        else:
            misc_al.append(f)
            assigned.add(f)

    for f in al_files:
        if f not in assigned:
            misc_al.append(f)

    return all_prs, misc_al


# ---------------------------------------------------------------------------
# Step 8: Output — text waves
# ---------------------------------------------------------------------------

def pr_label(info):
    return f"{info['directory']}/{info['name']}"


def format_waves(all_prs, deps, waves, branch_wide, misc_al, partial_cpp):
    lines = []
    lines.append("=" * 80)
    lines.append("PR PLAN — claude-testing-cleanup")
    lines.append("=" * 80)
    lines.append("")
    lines.append("Dependency rule: header-to-header includes only.")
    lines.append("PRs within the same WAVE can be submitted simultaneously.")
    lines.append("al/ files listed under the PR that motivated the change.")
    lines.append("")

    lines.append("━" * 80)
    lines.append("PREREQUISITE PRs (no source deps — submit anytime)")
    lines.append("━" * 80)
    lines.append("")

    lines.append("PR [infra-docs]:")
    for f in sorted(branch_wide):
        lines.append(f"  {f}")
    lines.append("")

    if misc_al:
        lines.append("PR [infra-al]: (no src/ users among changed files)")
        for f in sorted(misc_al):
            lines.append(f"  {f}")
        lines.append("")

    if partial_cpp:
        lines.append("PR [partial]: (review manually — partial edits, no new class header)")
        for f in sorted(partial_cpp):
            lines.append(f"  {f}")
        lines.append("")

    for wave_idx, wave in enumerate(waves, 1):
        lines.append("━" * 80)
        lines.append(f"WAVE {wave_idx}  ({len(wave)} PR{'s' if len(wave) != 1 else ''})")
        lines.append("━" * 80)
        lines.append("")

        for pk in wave:
            info = all_prs[pk]
            my_deps = sorted(pr_label(all_prs[d]) for d in deps.get(pk, set()))
            merged = info.get("merged_from", [])

            lines.append(f"  ┌─ {pr_label(info)}")
            if merged:
                lines.append(f"  │  [cycle: {', '.join(Path(k).name for k in merged)}]")
            if my_deps:
                lines.append(f"  │  needs: {', '.join(my_deps)}")
            for f in info["files"]:
                tag = " [al]" if f.startswith("lib/al/") else ""
                lines.append(f"  │    {f}{tag}")
            lines.append("  └─")
            lines.append("")

    impl_count = sum(1 for i in all_prs.values() if i["has_cpp"])
    lines.append("━" * 80)
    lines.append(f"TOTAL: {impl_count} impl PRs across {len(waves)} waves "
                 f"(+ 2-3 infra PRs)")
    lines.append("━" * 80)
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Step 9: DOT graph output




# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="pr_dependency_tree.txt")
    parser.add_argument("--json", default="pr_dependency_tree.json")
    args = parser.parse_args()

    def log(msg):
        print(msg, file=sys.stderr)

    log("Collecting changed files...")
    changed = get_changed_files()
    log(f"  {len(changed)} files")

    log("Classifying...")
    branch_wide, al_files, partial_cpp, src_files = classify(changed)

    log("Building class PRs...")
    class_prs = build_class_prs(src_files)
    log(f"  {len(class_prs)} initial")

    log("Merging header-only PRs...")
    before = len(class_prs)
    class_prs, orphaned = merge_header_only_prs(class_prs)
    partial_cpp.extend(orphaned)
    log(f"  {before} -> {len(class_prs)} ({before - len(class_prs)} merged)")

    log("Finding cycles (SCC on .h-only graph — only true header cycles require same PR)...")
    hdr_deps = build_dep_graph(class_prs, headers_only=True)
    all_prs, _ = merge_cycles(class_prs, hdr_deps)
    n_cycles = sum(1 for i in all_prs.values() if i.get("merged_from"))
    log(f"  {len(all_prs)} PRs ({n_cycles} header-cycle-merged)")

    log("Building ordering graph (h-deps + cpp-deps, removing back-edges that cause cycles)...")
    full_deps = build_dep_graph(all_prs, headers_only=False)
    # Remove edges that create cycles (these come from .cpp back-edges only, since
    # header cycles were already merged above). We iteratively strip back-edges until
    # the graph is a DAG, preserving as many ordering constraints as possible.
    deps = break_cycles_greedily(all_prs, full_deps)
    log(f"  {sum(len(v) for v in deps.values())} ordering edges ({sum(len(v) for v in full_deps.values()) - sum(len(v) for v in deps.values())} back-edges removed)")

    log("Computing waves...")
    waves = compute_waves(all_prs, deps)
    log(f"  {len(waves)} waves")

    log("Assigning al/ files...")
    all_prs, misc_al = assign_al_files(al_files, all_prs, waves)
    n_with_al = sum(1 for i in all_prs.values() if any(f.startswith("lib/al") for f in i["files"]))
    log(f"  {n_with_al} PRs got al files, {len(misc_al)} misc")

    log("Writing text output...")
    with open(args.output, "w") as fh:
        fh.write(format_waves(all_prs, deps, waves, branch_wide, misc_al, partial_cpp))

    log("Writing JSON...")
    out = {"waves": []}
    for wave_idx, wave in enumerate(waves, 1):
        out["waves"].append({
            "wave": wave_idx,
            "prs": [{
                "key": pk,
                "name": all_prs[pk]["name"],
                "directory": all_prs[pk]["directory"],
                "files": all_prs[pk]["files"],
                "depends_on": sorted(deps.get(pk, set())),
                "cycle_merged": all_prs[pk].get("merged_from", []),
            } for pk in wave]
        })
    with open(args.json, "w") as fh:
        json.dump(out, fh, indent=2)

    print()
    print(f"=== {len(all_prs)} PRs across {len(waves)} waves ===")
    for i, wave in enumerate(waves, 1):
        print(f"  Wave {i:2d}: {len(wave):3d} PRs")
    print(f"\nFiles: {args.output}  {args.json}")


if __name__ == "__main__":
    main()
