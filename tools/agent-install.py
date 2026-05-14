#!/usr/bin/env python3
"""
Install OdysseyDecomp agent and MCP server for an AI coding tool.

Usage:
  tools/agent-install.py <ida|ghidra> <opencode>
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Protocol


ROOT = Path(__file__).resolve().parent.parent


class AgentToolInstaller(Protocol):
    name: str

    @staticmethod
    def config_dir() -> Path:
        ...

    @staticmethod
    def edit_config(server_entry: dict) -> None:
        ...


class OpenCodeInstaller:
    name = "opencode"

    @staticmethod
    def config_dir() -> Path:
        env = os.environ.get("OPENCODE_CONFIG_PATH")
        if env:
            p = Path(env)
            if p.exists():
                return p

        try:
            r = subprocess.run(
                ["opencode", "config", "path"],
                capture_output=True, text=True, timeout=5,
            )
            if r.returncode == 0 and r.stdout.strip():
                p = Path(r.stdout.strip())
                if p.exists():
                    return p.parent if p.is_file() else p
        except (FileNotFoundError, subprocess.TimeoutExpired):
            pass

        xdg = Path.home() / ".config" / "opencode"
        if xdg.exists():
            return xdg

        legacy = Path.home() / ".opencode"
        if legacy.exists():
            return legacy

        raise SystemExit(
            f"Could not find OpenCode config directory.\n"
            f"Checked: $OPENCODE_CONFIG_PATH, `opencode config path`, {xdg}, {legacy}"
        )

    @staticmethod
    def edit_config(server_entry: dict) -> None:
        cfg_dir = OpenCodeInstaller.config_dir()
        cfg_path = cfg_dir / "opencode.json"
        if not cfg_path.exists():
            cfg_path = cfg_dir / "opencode.jsonc"
        if not cfg_path.exists():
            raise SystemExit(f"No opencode config at {cfg_dir / 'opencode.json'}")

        raw = cfg_path.read_text()
        try:
            cfg = json.loads(raw)
        except json.JSONDecodeError as e:
            raise SystemExit(f"Failed to parse {cfg_path}: {e}")

        if "mcp" not in cfg:
            cfg["mcp"] = {}
        cfg["mcp"]["odyssey"] = server_entry

        cfg_path.write_text(json.dumps(cfg, indent=2) + "\n")
        print(f"  Updated {cfg_path}")


AGENT_TOOLS: dict[str, type[AgentToolInstaller]] = {
    "opencode": OpenCodeInstaller,
}

DISASSEMBLERS = {
    "ida": {
        "agent_file": "AGENT-IDA.md",
    },
    "ghidra": {
        "agent_file": "AGENT-Ghidra.md",
    },
}


def main():
    parser = argparse.ArgumentParser(
        description="Install OdysseyDecomp agent and MCP server for an AI coding tool"
    )
    parser.add_argument(
        "disassembler",
        choices=list(DISASSEMBLERS),
        help="Disassembler to target (ida or ghidra)",
    )
    parser.add_argument(
        "tool",
        choices=list(AGENT_TOOLS),
        help="AI coding tool to install for",
    )
    args = parser.parse_args()

    dis = DISASSEMBLERS[args.disassembler]
    installer = AGENT_TOOLS[args.tool]

    try:
        subprocess.run(
            [sys.executable, "-c", "from mcp.server.fastmcp import FastMCP"],
            capture_output=True, timeout=5,
        )
    except (subprocess.CalledProcessError, FileNotFoundError):
        raise SystemExit(
            "Error: 'mcp' package not found. Install with: pip install mcp"
        )

    if not shutil.which("clangd"):
        print("Warning: clangd not found on PATH. The clangd_check tool won't work.")
        print("  Install clangd (e.g. apt install clangd or pkg install llvm).\n")

    agent_src = ROOT / "docs" / "llm" / dis["agent_file"]
    mcp_src = ROOT / "tools" / "odyssey-mcp.py"
    hypa_src = ROOT / "tools" / "hypa.py"
    matching_src = ROOT / "docs" / "llm" / "MATCHING.md"

    for path, label in [
        (agent_src, "Agent file"),
        (mcp_src, "MCP server"),
        (hypa_src, "hypa.py"),
        (matching_src, "MATCHING.md"),
    ]:
        if not path.exists():
            raise SystemExit(f"{label} not found: {path}")

    cfg_dir = installer.config_dir()
    scripts_dir = cfg_dir / "scripts" / "odyssey"
    agents_dir = cfg_dir / "agents"

    scripts_dir.mkdir(parents=True, exist_ok=True)
    agents_dir.mkdir(parents=True, exist_ok=True)

    agent_dst = agents_dir / "OdysseyDecomp.md"
    shutil.copy2(agent_src, agent_dst)
    print(f"  Copied {agent_src} -> {agent_dst}")

    mcp_dst = scripts_dir / "odyssey-mcp.py"
    shutil.copy2(mcp_src, mcp_dst)
    print(f"  Copied {mcp_src} -> {mcp_dst}")

    hypa_dst = scripts_dir / "hypa.py"
    shutil.copy2(hypa_src, hypa_dst)
    print(f"  Copied {hypa_src} -> {hypa_dst}")

    matching_dst = scripts_dir / "MATCHING.md"
    shutil.copy2(matching_src, matching_dst)
    print(f"  Copied {matching_src} -> {matching_dst}")

    mcp_dst.chmod(mcp_dst.stat().st_mode | 0o111)

    server_entry = {
        "type": "local",
        "command": ["python3", str(mcp_dst), str(ROOT)],
        "enabled": True,
    }

    installer.edit_config(server_entry)

    print()
    print("Done. OdysseyDecomp agent installed to your OpenCode config.")
    print("Restart or reload your agent tool for changes to take effect.")


if __name__ == "__main__":
    main()
