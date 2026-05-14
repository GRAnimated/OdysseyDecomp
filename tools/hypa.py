#!/usr/bin/env python3
"""
Usage:
  tools/hypa.py <function_name> [function_name ...]
  cat pseudocode.txt | tools/hypa.py
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).resolve().parent.parent
FILE_LIST_PATH = ROOT / "data" / "file_list.yml"
SRC_DIR = ROOT / "src"
LIB_DIR = ROOT / "lib"

CXX_KEYWORDS = frozenset({
    "if", "else", "for", "while", "do", "switch", "case", "return", "throw",
    "try", "catch", "new", "delete", "sizeof", "decltype", "typeof",
    "static_cast", "dynamic_cast", "reinterpret_cast", "const_cast",
    "and", "or", "not", "typeid", "noexcept", "alignof",
})


def resolve_include(file_path: Path) -> str:
    try:
        rel = file_path.relative_to(ROOT / "src").as_posix()
        return f'#include "{rel}"'
    except ValueError:
        pass
    try:
        rel = file_path.relative_to(ROOT / "lib" / "al").as_posix()
        return f'#include "{rel}"'
    except ValueError:
        pass
    for lib_sub in ["sead", "agl", "eui", "NintendoSDK", "NintendoSDK-NEX"]:
        try:
            rel = file_path.relative_to(LIB_DIR / lib_sub / "include").as_posix()
            return f"#include <{rel}>"
        except ValueError:
            continue
    try:
        rel = file_path.relative_to(ROOT).as_posix()
        return f"#include <{rel}>"
    except ValueError:
        return f'#include "{file_path.name}"'


def demangle_bulk(mangled_names: list) -> dict:
    if not mangled_names:
        return {}
    r = subprocess.run(["c++filt"], input="\n".join(mangled_names),
                       capture_output=True, text=True)
    demangled_lines = r.stdout.strip().split("\n")
    result = {}
    for mangled, demangled in zip(mangled_names, demangled_lines):
        result[mangled] = demangled
    return result


def normalize_name(name: str) -> str:
    paren = name.find("(")
    if paren != -1:
        name = name[:paren]
    return name.strip()


def parse_file_list():
    mangled_info = {}
    current_obj = None
    current_offset = None
    current_size = None
    current_status = None

    with open(FILE_LIST_PATH) as f:
        for line in f:
            if line and line[0] not in " \t\n\r":
                stripped = line.strip()
                if stripped.endswith(".o:"):
                    current_obj = stripped[:-1]
                continue
            s = line.strip()
            if not s:
                continue
            if s.startswith("- offset:"):
                current_offset = int(s.split()[-1], 16)
                continue
            if s.startswith("size:"):
                current_size = int(s.split()[-1])
                continue
            if s.startswith("label:"):
                parts = s.split(None, 1)
                if len(parts) >= 2:
                    val = parts[1].strip()
                    if val.startswith("_Z"):
                        mangled_info[val] = {
                            "obj": current_obj, "offset": current_offset,
                            "size": current_size, "status": current_status,
                        }
                continue
            if s.startswith("- ") and len(s) > 2 and s[2:].startswith("_"):
                val = s[2:].strip()
                if val.startswith("_Z"):
                    mangled_info[val] = {
                        "obj": current_obj, "offset": current_offset,
                        "size": current_size, "status": current_status,
                    }
                continue
            if s.startswith("status:"):
                current_status = s.split()[-1]
                continue

    demangled_map = demangle_bulk(list(mangled_info.keys()))
    demangled_to_mangled = {}
    for mangled, demangled in demangled_map.items():
        key = normalize_name(demangled)
        if key not in demangled_to_mangled:
            demangled_to_mangled[key] = []
        demangled_to_mangled[key].append(mangled)

    return mangled_info, demangled_to_mangled, demangled_map


def is_func_decl_line(s: str) -> bool:
    if not s or s[0] in "#/*":
        return False
    skip_prefixes = ("using ", "typedef ", "static_assert", "namespace ", "template<",
                     "extern ", "enum ", "class ", "struct ", "if ", "for ", "while ",
                     "switch ", "else ", "constexpr ",
                     "#define", "#if", "#endif", "#else", "#elif", "#pragma", "#include")
    if s.startswith(skip_prefixes):
        return False
    if "operator" in s or "(" not in s or ")" not in s:
        return False
    if not (s.endswith(";") or "{" in s):
        return False
    op = s.index("(")
    eq = s.find("=")
    if eq != -1 and eq < op:
        return False
    cp = s.rindex(")")
    if cp < op or "=" in s[cp:]:
        return False
    if "{" in s and s.index("{") < op:
        return False
    return True


def extract_func_name(text: str) -> str:
    s = text.split("{")[0].rstrip(";").strip()
    for kw in ("virtual ", "static ", "inline ", "constexpr ", "explicit ", "friend "):
        if s.startswith(kw):
            s = s[len(kw):].lstrip()
    for kw in (" const", " override", " noexcept", " final", " = 0", " = delete", " = default",
               " volatile", " &", " &&"):
        while s.endswith(kw):
            s = s[:-len(kw)].rstrip()
    np = s[:s.index("(")].strip()
    idx = np.rfind(" ")
    if idx != -1:
        np = np[idx + 1:]
    while np.endswith("*") or np.endswith("&"):
        np = np[:-1]
        idx = np.rfind(" ")
        if idx != -1:
            np = np[idx + 1:]
    return np


def make_qualified_output(text: str, qualified_name: str) -> str:
    s = text.split("{")[0].rstrip(";").strip()
    for kw in ("virtual ", "static ", "inline ", "constexpr ", "explicit ", "friend "):
        if s.startswith(kw):
            s = s[len(kw):].lstrip()
    for kw in (" const", " override", " noexcept", " final", " = 0", " = delete", " = default",
               " volatile", " &", " &&"):
        while s.endswith(kw):
            s = s[:-len(kw)].rstrip()
    oi = s.index("(")
    before = s[:oi].strip()
    after = s[oi:]
    if " " in before:
        ret, fname = before.rsplit(None, 1)
        while fname.endswith("*") or fname.endswith("&"):
            ret += fname[-1]
            fname = fname[:-1]
        return f"{ret} {qualified_name}{after};"
    return f"{qualified_name}{after};"


def build_decl_index():
    ns_pat = re.compile(r'^namespace\s+(\w+)\s*\{')
    class_keyword_pat = re.compile(r'^(?:class|struct)\s+(\w+)')
    template_pat = re.compile(r'^\s*template\s*<')

    index = defaultdict(list)

    for root_dir in [SRC_DIR, LIB_DIR]:
        for h_path in sorted(root_dir.rglob("*.h")):
            inc = resolve_include(h_path)
            try:
                text = h_path.read_text(errors="replace")
            except Exception:
                continue
            current_ns = []
            current_classes = []
            pending_class = None
            brace_depth = 0
            rel_path = str(h_path.relative_to(ROOT))
            for i, raw_line in enumerate(text.split("\n"), 1):
                line = raw_line.split("//")[0].strip()
                if not line:
                    continue
                if template_pat.match(line):
                    continue
                if line == "}" or line == "};":
                    brace_depth -= 1
                    if brace_depth < 0:
                        brace_depth = 0
                    if current_classes and brace_depth < len(current_ns) + len(current_classes):
                        current_classes.pop()
                    elif current_ns and brace_depth < len(current_ns):
                        current_ns.pop()
                    pending_class = None
                    continue
                ns_m = ns_pat.match(line)
                if ns_m:
                    current_ns.append(ns_m.group(1))
                    brace_depth += 1
                    continue
                cls_m = class_keyword_pat.match(line)
                if cls_m:
                    if "{" in line:
                        current_classes.append(cls_m.group(1))
                        brace_depth += 1
                        pending_class = None
                    else:
                        pending_class = cls_m.group(1)
                    continue
                if "{" in line and pending_class and "(" not in line:
                    current_classes.append(pending_class)
                    brace_depth += 1
                    pending_class = None
                if not is_func_decl_line(line):
                    continue
                func_name = extract_func_name(line)
                if not func_name:
                    continue
                qual_parts = current_ns + current_classes
                prefix = "::".join(qual_parts) + "::" if qual_parts else ""
                qualified = prefix + func_name
                decl = {
                    "include": inc,
                    "file": rel_path,
                    "line": i,
                    "text": line,
                    "qualified": qualified,
                }
                index[func_name].append(decl)
                if prefix:
                    index[qualified].append(decl)
                    for comp in qual_parts:
                        index[comp + "::" + func_name].append(decl)
                left = line.split("(")[0].rsplit(None, 1)
                last_token = left[-1] if left else ""
                if last_token and last_token != func_name:
                    index[last_token].append(decl)
    return index


def extract_names_from_text(text: str) -> list:
    pat = re.compile(r'(?<![.\w>])'
                     r'(\w+(?:::\w+)*)'
                     r'\s*\(')
    seen = set()
    result = []
    for m in pat.finditer(text):
        name = m.group(1)
        if name.lower() in CXX_KEYWORDS:
            continue
        if name not in seen:
            seen.add(name)
            result.append(name)
    return result


def obj_to_header_hint(obj: str) -> str:
    if obj.endswith(".o"):
        obj = obj[:-2]
    stem = obj.split("/")[-1]
    base = obj.rsplit("/", 1)[0] if "/" in obj else ""
    h_name = stem + ".h"
    if base:
        return base + "/" + h_name
    return h_name


def main():
    parser = argparse.ArgumentParser(
        description="Quick function signature & include path retriever")
    parser.add_argument("--project-root", type=Path, default=None,
                        help="Override project root (for running outside repo)")
    parser.add_argument("-f", "--fuzzy", action="store_true",
                        help="Fuzzy match: show all functions starting with the given name")
    parser.add_argument("names", nargs="*", help="Function names to look up")
    args = parser.parse_args()

    global ROOT, FILE_LIST_PATH, SRC_DIR, LIB_DIR
    if args.project_root:
        ROOT = args.project_root.resolve()
        FILE_LIST_PATH = ROOT / "data" / "file_list.yml"
        SRC_DIR = ROOT / "src"
        LIB_DIR = ROOT / "lib"

    names = args.names
    pseudocode_input = not sys.stdin.isatty()

    if names:
        pass
    elif pseudocode_input:
        text = sys.stdin.read()
        names = extract_names_from_text(text)
        if not names:
            print("No function-like tokens found in input.", file=sys.stderr)
            sys.exit(1)
    else:
        parser.print_help()
        sys.exit(1)

    mangled_info, demangled_to_mangled, demangled_map = parse_file_list()
    decl_index = build_decl_index()

    found_by_file = defaultdict(list)
    unknown = []

    for raw_name in names:
        lookup_keys = [raw_name]
        if raw_name.startswith("_Z"):
            try:
                r = subprocess.run(["c++filt", raw_name],
                                   capture_output=True, text=True)
                if r.returncode == 0:
                    demangled = r.stdout.strip()
                    lookup_keys.append(normalize_name(demangled))
            except Exception:
                pass
        else:
            if raw_name.startswith("::"):
                raw_name = raw_name[2:]
            lookup_keys.append(raw_name)

        matched = False
        for key in lookup_keys:
            if key in decl_index:
                for entry in decl_index[key]:
                    found_by_file[entry["include"]].append(entry)
                matched = True
            if matched:
                break

        if not matched and args.fuzzy:
            for key in list(decl_index.keys()):
                if key.startswith(raw_name):
                    for entry in decl_index[key]:
                        found_by_file[entry["include"]].append(entry)
                    matched = True

        if not matched:
            unknown.append(raw_name)

    for inc in sorted(found_by_file.keys()):
        entries = found_by_file[inc]
        seen = set()
        out = []
        for e in entries:
            dedup_key = (e["line"], e["qualified"])
            if dedup_key not in seen:
                seen.add(dedup_key)
                out.append((e["line"], e["qualified"], e["text"]))
        if out:
            print(inc)
            for line_no, qual, raw_text in sorted(out):
                sig = make_qualified_output(raw_text, qual)
                print(f"{line_no}: {sig}")
            print()

    for name in unknown:
        mangled_candidates = []
        check_key = name
        if not name.startswith("_Z"):
            for dname, mlist in demangled_to_mangled.items():
                if (check_key == dname or
                    dname.endswith("::" + check_key) or
                    dname.startswith(check_key + "(")):
                    mangled_candidates.extend(mlist)
        else:
            mangled_candidates = [name]

        if mangled_candidates:
            seen_objs = set()
            for mc in mangled_candidates:
                info = mangled_info.get(mc)
                if info and info["obj"] not in seen_objs:
                    seen_objs.add(info["obj"])
                    hint = obj_to_header_hint(info["obj"])
                    full_sig = demangled_map.get(mc, mc)
                    print(f"No declaration for {name}")
                    print(f'  Suggested: #include "{hint}"')
                    print(f"  {full_sig};")
                    print()
                    break
        else:
            print(f"Unknown symbol: {name}")
            print()


if __name__ == "__main__":
    main()
