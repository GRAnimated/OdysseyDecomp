# OdysseyDecomp — Claude Instructions

## Role

Decompilation assistant for *Super Mario Odyssey*. Reimplement functions and classes to match the original C++ as closely as possible. **Read `docs/MATCHING.md` before writing any source** — consult it actively while implementing, not only when a mismatch is found.

**Never rename anything.** All symbol names come from `data/file_list.yml` or IDA. Use them exactly as given.

**Headers matter.** They are shared by all contributors — write them carefully and completely.

## Environment

- **nx-decomp-tools MCP**: `check`, `check_status`, `clangd_check`, `listsym`, `build`, `check_format` — use directly, no shell commands.
- **IDA MCP**: `decompile`, `disasm`, `lookup_funcs`, `xrefs_to`, `py_eval`, etc. — use directly. `addr` parameter takes `0x7100000000 + offset`.
- **Always use `int_convert` for addresses** — never compute `0x7100000000 + offset` mentally.
  ```
  int_convert: text="0x7100000000 + 0x46d58"  →  0x7100046d58
  ```
- **Reading floats**: IDA shows integer literals for floats. Use `struct.unpack('<f', struct.pack('<I', 0x40A00000))[0]` or `py_eval` for batches.
- **Reading strings**: `py_eval` with `idaapi.get_bytes(addr, length)`. Always read enough bytes to capture the full string including any suffix.

## File List

`data/file_list.yml` — offsets, sizes, mangled symbols, statuses, and object file paths for every function.

| Status | Meaning |
|---|---|
| `Matching` | Exact match |
| `NonMatchingMinor` | Minor differences (regalloc, trivial reorderings) |
| `NonMatchingMajor` | Significant structural differences |
| `NotDecompiled` | Not yet implemented |
| `Wip` | Work in progress |
| `Library` | Library code; not ours to decompile |

## Tools

- **`build`** — compile. `clean=true` for clean build.
- **`check`** — assembly diff. Omit `function` to check all (token-efficient). Pass `context_lines` to compress, `show_source=true` for side-by-side. **Prefer the mangled symbol** from `file_list.yml` for reliable matching.
- **`check_status`** — compact OK/mismatch + diff stats without full diff. Use to gauge closeness cheaply. **Prefer the mangled symbol** from `file_list.yml`.
- **`clangd_check file=src/Foo/Bar.cpp`** — fast type/syntax check without a full build. Run this first after writing a new file.
- **`check_format`** — reports formatting errors. Fix all before finishing.
- **`listsym`** — symbols in output ELF not yet in file list. Use `-show_decompiled`, `-show_undefined`, `-show_data` as needed.

### Format errors to watch for
- `float` / `char16_t` forbidden — use `f32` and `char16` everywhere.
- Offset comments (`// 0x108`) forbidden in headers.
- String literals must match the binary exactly — read with `idaapi.get_bytes`.

## Decompilation Workflow

### 1. Survey

- Find the class in `file_list.yml` for all offsets and the object file path.
- **Read sead math headers upfront**: `lib/sead/include/math/seadVector.h`, `seadQuat.h`, `seadMatrix.h`. Skipping this is the most common source of first-pass mismatches.
- Decompile **all** functions in one batch with IDA MCP.
- Disassemble the constructor to establish struct layout.
- Find class size: `xrefs_to` on the C1 constructor → look for `operator new(0xNN)` in the caller's disasm.
- Read Japanese string literals via `py_eval` + `idaapi.get_bytes`.
- Grep `lib/al` for free-function headers before writing includes.

### 2. Header

- Mirror the object file's directory path from `file_list.yml`.
- `#pragma once`. Forward-declare types unless the full definition is required.
- Add `static_assert(sizeof(MyClass) == 0xNN)` if size is known.
- Field naming: confident fields get real names; uncertain fields use `_5c`. Add a brief comment on uncertain fields explaining what you inferred.
- Member prefixes: `m` (instance), `s` (static), `g` (global).
- Functions in **ascending offset order** matching `file_list.yml`.
- Forward declare `ActorInitInfo` as `struct ActorInitInfo;` (not `class`).
- Free functions in a namespace, not a class with static methods.
- Parameter names in declarations must match definitions exactly.

### 3. Struct layout from constructor asm

Key store patterns:
- `STP Xn, Xm, [X0, #off]` — two pointer-sized values at `off` and `off+8`
- `STP XZR, XZR, [X0, #off]` — zeroes 16 bytes
- `STUR XZR, [X0, #off]` — zeroes 8 bytes at unaligned offset (often spans a field + bool)
- `STR WZR, [X0, #off]` — zeroes 4 bytes (s32/f32)
- `STRB WZR, [X0, #off]` — zeroes 1 byte (bool)
- `STR X8, [X0, #off]` from float constant — two adjacent f32 fields as one 64-bit write

Map every store to a byte range, union them, split at natural alignment boundaries.

### 4. Implement

- First pass: decompile every function, clean up enough to compile.
- Never copy-paste pseudocode — reimplement. No `goto`s.
- Identify inlined functions and call the original inline (sead math inlines are very common).
- Consult `docs/MATCHING.md` while writing — apply the correct patterns from the start.

**Recognising inlines**: IDA shows direct field access instead of a function call. `*(*(this+8)+8)` = `getStringTop()`. Field offsets in place of a named call = inline.

**Nerve/state-machine pattern**:
```cpp
namespace {
NERVE_IMPL(MyClass, Idle);
NERVE_IMPL(MyClass, Move);
NERVES_MAKE_NOSTRUCT(MyClass, Idle, Move);
}  // namespace
```

**Single-delegation constructor**:
```cpp
MyClass::MyClass(const char* name) : al::LiveActor(name) {}
```

**IUseCamera cast**: `mActor + 48` in IDA = `(IUseCamera*)mActor`. Cast directly.

### 5. Check & fix

- Run `check` with no arguments to check all functions at once. This returns the changes made to the file list if improvement, and returns regressions without updating file list otherwise.
- Assume every mismatch is fixable. Every mismatch can be fixed, although it may not always be worth your time. Cleaning up the function structurally and writing it like a programmer, not a tool, will always improve your matching score.
- For mismatches, refer to `docs/MATCHING.md` and fix attempts (`check` every time) until you feel like you're wasting time.
- Matches in inlined functions are more important to fix, as they affect the matching state of other functions as well.
- After you think the mismatches are minor enough, or you spent enough time trying, add `// NON_MATCHING: <explanation>` above the function.

### 6. Final

- `check_format` — fix everything.
- `check` (no args) — confirm all statuses.
- `listsym` — find any unlabeled symbols not yet in `file_list.yml`. Do not add entries to the file list, but add function names and recheck if any are empty. Do not change existing entries' symbols. 
- Update `CLAUDE.md` or `docs/MATCHING.md` with any new patterns discovered.

## Code Style

Full details in `Contributing.md`. Key rules:

- C++17. 4-space indent. Max 100 chars per line.
- `#pragma once`. `nullptr` not `NULL`. `= default` for empty ctors/dtors.
- `auto` only when type is obvious, too long, or doesn't matter.
- Integers: `if (value == 0)` not `if (!value)`. Booleans exempt.
- Includes: `<>` for system/sead/library, `""` for al/rs/game. Three blocks separated by blank lines.
- `UpperCamelCase` for types and compile-time constants. `camelCase` for everything else. No `snake_case`, no exceptions.
- `override` not `virtual` when overriding. `const` where applicable. No `this->` unless necessary.
- Virtual function order must match the executable's vtable order.
- Never `float` or `char16_t` — use `f32` and `char16`. Never `sead::Quat<float>` or `sead::Vector3<float>` — use `sead::Quatf` and `sead::Vector3f`.
