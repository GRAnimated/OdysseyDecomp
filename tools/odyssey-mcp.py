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
    args = [sys.executable, str(HYPA)]
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
    context_lines: Optional[int] = None,
    show_source: bool = False,
) -> str:
    """
    Assembly diff. Omit function to check all.

    Args:
        function: MangLED or demangled symbol name. None checks all.
        context_lines: Number of context lines for the diff (-U N).
        show_source: Show source alongside assembly (-c).
    """
    args = ["tools/check", "--no-pager", "--format=plain"]
    if context_lines is not None:
        args.extend(["-U", str(context_lines)])
    if show_source:
        args.append("-c")
    if function:
        args.append(function)
    output = _run(args, timeout=5)
    return output


@mcp.tool()
def check_status(function: str) -> str:
    """
    Compact OK/mismatch status with diff stats for one function.

    Args:
        function: MangLED or demangled symbol name.
    """
    raw = _run(
        ["tools/check", "--no-pager", "--format=plain", "--always-diff", function],
        timeout=30,
    )

    if "mismatch" in raw and "OK" not in raw.split("mismatch")[0].split("\n")[-1]:
        verdict = "mismatch"
        m = re.search(r"mismatch at [0-9a-fx]+: (.+)", raw)
        reason = m.group(1).strip() if m else "unknown"
    else:
        verdict = "OK"
        reason = ""

    _DIFF_LINE = re.compile(
        r"^[0-9a-f]+:\s+\S.*?([ s|r<>])\s+(?:[0-9a-f]+:\s+\S.*)?$"
    )

    counts = {"match": 0, "s": 0, "|": 0, "r": 0, "<": 0, ">": 0}
    for line in raw.splitlines():
        m = _DIFF_LINE.match(line)
        if m:
            marker = m.group(1)
            if marker == " ":
                counts["match"] += 1
            elif marker in counts:
                counts[marker] += 1

    total = sum(counts.values())
    effectively = counts["match"] + counts["r"]

    if verdict == "OK":
        return f"{function}: OK ({effectively} matching, {counts['r']} regswap)"

    pct = int(100 * effectively / total) if total > 0 else 0
    parts = []
    if counts["|"]: parts.append(f'{counts["|"]} changed')
    if counts["s"]: parts.append(f'{counts["s"]} imm/offset')
    if counts["r"]: parts.append(f'{counts["r"]} regswap')
    if counts["<"]: parts.append(f'{counts["<"]} deleted')
    if counts[">"]: parts.append(f'{counts[">"]} added')
    return (
        f"{function}: mismatch ({reason})\n"
        f"  {effectively}/{total} match ({pct}%), {', '.join(parts)}"
    )


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
