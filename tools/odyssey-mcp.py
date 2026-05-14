#!/usr/bin/env python3

import re
import subprocess
import sys
from pathlib import Path
from typing import Optional

from mcp.server.fastmcp import FastMCP

REPO = Path(sys.argv[1]).resolve()
SCRIPTS = Path(__file__).resolve().parent
HYPA = SCRIPTS / "hypa.py"
_ANSI = re.compile(r"\x1b(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])")

mcp = FastMCP("odyssey")


_DIFF_MARKER = re.compile(r"^[0-9a-f]+:\s+\S.*?([ s|r<>])\s+(?:[0-9a-f]+:\s+\S.*)?$")


def _compress(text: str, context: int) -> str:
    lines = text.splitlines()
    if not lines:
        return text

    markers = []
    for line in lines:
        m = _DIFF_MARKER.match(line)
        markers.append(m.group(1) if m else None)

    result = []
    i = 0
    while i < len(lines):
        if markers[i] == " ":
            start = i
            while i < len(lines) and markers[i] == " ":
                i += 1
            streak = i - start
            if streak > context * 2 + 1:
                result.extend(lines[start:start + context])
                result.append("...")
                result.extend(lines[i - context:i])
            else:
                result.extend(lines[start:i])
        else:
            result.append(lines[i])
            i += 1

    return "\n".join(result)


def _run(args: list[str], timeout: int = 120) -> str:
    result = subprocess.run(
        args, cwd=REPO, stdin=subprocess.DEVNULL,
        capture_output=True, text=True, timeout=timeout,
    )
    return _ANSI.sub("", result.stdout + result.stderr)


@mcp.tool()
def hypa(input: str, fuzzy: bool = False) -> str:
    """
    Resolve function declarations and include paths.
    Pass pseudocode (contains '(') to extract function names automatically,
    or pass space-separated function names directly.

    Args:
        input: Function names or IDA pseudocode.
        fuzzy: Enable fuzzy matching (show all functions starting with the given name).
    """
    args = [sys.executable, str(HYPA), "--project-root", str(REPO)]
    if fuzzy:
        args.append("-f")
    if "(" in input:
        result = subprocess.run(
            args, cwd=REPO, input=input, capture_output=True, text=True, timeout=30,
        )
        return _ANSI.sub("", result.stdout + result.stderr)
    else:
        args.extend(input.split())
        return _run(args, timeout=30)


@mcp.tool()
def build(clean: bool = False) -> str:
    """Compile the project. clean=True for clean build."""
    return _run(["tools/build.py"] + (["--clean"] if clean else []), timeout=300)


@mcp.tool()
def check(
    function: Optional[str] = None,
    functions: Optional[list[str]] = None,
    context_lines: Optional[int] = None,
    show_source: bool = False,
) -> str:
    """
    Assembly diff. Omit function/functions to check all.

    Args:
        function: Single mangled or demangled symbol name.
        functions: Multiple symbol names to check in batch.
        context_lines: Number of context lines around non-matching parts.
        show_source: Show source alongside assembly (-c).
    """
    targets: list[Optional[str]] = list(functions) if functions else [function] if function else [None]
    base = ["tools/check", "--no-pager", "--format=plain"]
    if show_source:
        base.append("-c")
    results = []
    for target in targets:
        args = base + ([target] if target else [])
        output = _run(args, timeout=5)
        if context_lines is not None:
            output = _compress(output, context_lines)
        results.append(output)
    return "\n".join(results) if len(results) > 1 else results[0]


@mcp.tool()
def check_status(
    function: Optional[str] = None,
    functions: Optional[list[str]] = None,
    filter: Optional[str] = None,
) -> str:
    """
    Compact OK/mismatch status with diff stats. Accepts one or multiple functions, or a filter.

    Args:
        function: Single mangled or demangled symbol name.
        functions: Multiple symbol names to check in batch.
        filter: Only show results for functions matching this text pattern.
    """
    if filter:
        raw = _run(["tools/check", "--no-pager", "--format=plain"], timeout=30)
        lines = raw.splitlines()
        matching = [l for l in lines if filter.lower() in l.lower()]
        return "\n".join(matching) if matching else f"(no functions matching '{filter}')"

    targets: list[str] = list(functions) if functions else ([function] if function else [])
    if not targets:
        return "check_status requires at least one function name"

    results = []
    for target in targets:
        raw = _run(
            ["tools/check", "--no-pager", "--format=plain", "--always-diff", target],
            timeout=30,
        )

        if "mismatch" in raw and "OK" not in raw.split("mismatch")[0].split("\n")[-1]:
            verdict = "mismatch"
            m = re.search(r"mismatch at [0-9a-fx]+: (.+)", raw)
            reason = m.group(1).strip() if m else "unknown"
        else:
            verdict = "OK"
            reason = ""

        counts = {"match": 0, "s": 0, "|": 0, "r": 0, "<": 0, ">": 0}
        for line in raw.splitlines():
            m = _DIFF_MARKER.match(line)
            if m:
                marker = m.group(1)
                if marker == " ":
                    counts["match"] += 1
                elif marker in counts:
                    counts[marker] += 1

        total = sum(counts.values())
        effectively = counts["match"] + counts["r"]

        if verdict == "OK":
            results.append(f"{target}: OK ({effectively} matching, {counts['r']} regswap)")
        else:
            pct = int(100 * effectively / total) if total > 0 else 0
            parts = []
            if counts["|"]: parts.append(f'{counts["|"]} changed')
            if counts["s"]: parts.append(f'{counts["s"]} imm/offset')
            if counts["r"]: parts.append(f'{counts["r"]} regswap')
            if counts["<"]: parts.append(f'{counts["<"]} deleted')
            if counts[">"]: parts.append(f'{counts[">"]} added')
            results.append(
                f"{target}: mismatch ({reason})\n"
                f"  {effectively}/{total} match ({pct}%), {', '.join(parts)}"
            )

    return "\n".join(results)


@mcp.tool()
def listsym(
    filter: Optional[str] = None,
    show_undefined: bool = False,
    show_data: bool = False,
    show_decompiled: bool = False,
) -> str:
    """
    List symbols in output ELF not yet in file list.

    Args:
        filter: Optional search string.
        show_undefined: Show undefined symbols (-u).
        show_data: Include data symbols (-d).
        show_decompiled: Show decompiled symbols in file list (-l).
    """
    args = ["tools/listsym"]
    if show_undefined: args.append("-u")
    if show_data: args.append("-d")
    if show_decompiled: args.append("-l")
    if filter: args.append(filter)
    return _run(args, timeout=30)


@mcp.tool()
def check_format() -> str:
    """Report formatting errors. Fix all before finishing."""
    return _run(["tools/check-format.py"], timeout=30)


@mcp.tool()
def clangd_check(file: str) -> str:
    """
    Fast type/syntax check without a full build.
    Requires compile_commands.json (from a prior build).

    Args:
        file: Path to .cpp or .h file relative to project root.
    """
    result = subprocess.run(
        ["clangd", f"--check={file}", "--compile-commands-dir=build"],
        cwd=REPO, stdin=subprocess.DEVNULL,
        capture_output=True, text=True, timeout=120,
    )
    raw = _ANSI.sub("", result.stderr)

    _DIAG = re.compile(r"^E\[\d{2}:\d{2}:\d{2}\.\d{3}\] ")
    _PREFIX = re.compile(r"^[IWE]\[\d{2}:\d{2}:\d{2}\.\d{3}\] ")
    _SUMMARY = re.compile(r"^I\[\d{2}:\d{2}:\d{2}\.\d{3}\] All checks completed")

    diags = []
    summary = None
    for line in raw.splitlines():
        if _SUMMARY.match(line):
            summary = _PREFIX.sub("", line)
        elif _DIAG.match(line):
            diags.append(_PREFIX.sub("", line))

    if summary is None:
        return f"clangd: no output (exit {result.returncode})"
    return "\n".join(diags + [summary])


if __name__ == "__main__":
    mcp.run()
