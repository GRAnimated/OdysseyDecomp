# OdysseyDecomp — Claude Instructions

## Role

You are a decompilation assistant for *Super Mario Odyssey*. Your job is to analyze and reimplement functions and classes as close as possible to the original C++ game code. When given a class to work on, work to fully match it using the tools described below.

**Headers matter.** They are shared by all contributors — write them carefully and completely. A good header makes the whole codebase easier to work on.

**Never rename anything.** All symbol names come from `data/file_list.yml` or from IDA. Use them exactly as given. Do not rename, abbreviate, or "clean up" any identifier that was not invented by you.

## Tools

### Build

```
tools/build.py
```

Use `--clean` if a clean build is necessary.

### Check (assembly diff)

```
tools/check                          # check every decompiled function
tools/check --no-pager namespace::func_name   # check a single function or symbol
```

- Use `-c` to show source code alongside assembly (more thorough, costs tokens).
- Use `-U <num>` to compress streaks of matching lines and focus on mismatches.
- Also accepts mangled symbols directly.
- The checker updates matching status in the file list.

### List symbols

```
tools/listsym <search>    # symbols in output ELF not in the file list
tools/listsym -d          # include data symbols
tools/listsym -u          # undefined (outgoing refs to unimplemented functions)
tools/listsym -l          # decompiled symbols (exist in file list)
```

### Format check

```
tools/check-format.py     # reports every formatting problem to fix before a class is done
```

### File list

`data/file_list.yml` — defines the offset, size, label (mangled symbol), and status of every function, and which object file it belongs to. Use this to find functions and to match source/header file directory structure.

Each function entry has a `status` field with one of these values:

| Status | Meaning |
|---|---|
| `Matching` | Assembly matches the original exactly |
| `NonMatchingMinor` | Minor differences only (regalloc, trivial reorderings) |
| `NonMatchingMajor` | Significant structural differences remain |
| `NotDecompiled` | Not yet implemented |
| `Wip` | Work in progress |
| `Library` | Comes from a library; not our code to decompile |

Update statuses as you work. `tools/check` does this automatically for functions you check individually. Verify against `file_list.yml` to ensure nothing was missed.

### IDA Pro (MCP)

ida-pro-mcp is your **main point of reference**. Key operations:

- `decompile` — decompile a function to pseudocode (use heavily during implementation)
- `disasm` — disassemble a function
- `lookup_funcs` / `list_funcs` — find functions by address or name
- `xrefs_to` — find callers of a function or address
- `xrefs_to_field` — find usages of a struct field
- `list_globals` / `get_global_value` — inspect globals
- `rename` — rename functions, locals, stack variables in IDA
- `set_type` — apply types to functions/globals/locals
- `declare_type` / `read_struct` / `search_structs` — manage struct types
- `set_comments` — annotate the decompiler view to aid analysis
- `int_convert` — **always use this** to convert number bases; never convert manually
- `find_regex` / `find` / `find_bytes` — search the binary

## Decompilation Workflow

### 1. Create the header first

- Mirror the object file's directory path from `file_list.yml` for all source/header files.
- Start with `#pragma once`.
- Fill in function declarations, includes, forward declarations, and inheritance.
- **Always forward declare** types in headers when you don't need the full definition. Only `#include` what is strictly required by the header itself. This reduces compile times and keeps headers clean for other contributors.
- If you know the class size (check `operator new` xrefs in the constructor), add it as padding using `void*` fields (can be an array) and add a `static_assert` at the bottom. Omit the assert if the size cannot be determined:

```cpp
static_assert(sizeof(MyClass) == 0x1D8);
```

- Only name fields you are confident about; use filler names like `_0x58` otherwise.
- Use `= nullptr` / `= 0` / `= false` initializers on member variables where appropriate.
- Member variables: `m` prefix + camelCase (e.g., `mAudioKeeper`). Static members: `s` prefix.
- Use scoped `enum class` for typed enumerations; define them inside the class if they belong to it.

### 2. Analyze fields thoroughly

- Examine the most important functions (often all of them) to find every field usage.
- Use IDA decompiler output for thoroughness; assembly for speed.
- Create the corresponding IDA struct type to clean up the decompiler output.
- Rename IDA variables and add comments to the decompiler view as you go.
- If IDA's argument types look wrong for a call, fix them to match the decomp's equivalent.

### 3. Implement the source file

- Do a **first pass**: decompile every function with `decompile`, clean up the output enough to compile.
- IDA may produce `goto`s, unrolled loops, and out-of-place logic — the original never used `goto`s. Reorder logic to be sensible without sacrificing the match.
- Do **not** copy-paste pseudocode. Reimplement it. Stubs are fine for unimplemented callees.
- Identify inlined functions and uninline them (call the original inline; let the compiler re-inline it). sead inlines are especially common — use math and vector inlines wherever applicable.
- If something doesn't exist in sead, do not add it.

**Nerve/state-machine pattern** (very common in this codebase): nerve declarations go in an anonymous namespace at the top of the `.cpp` file using `NERVE_IMPL` or `NERVE_ACTION_IMPL`, followed by `NERVES_MAKE_NOSTRUCT` or `NERVE_ACTIONS_MAKE_STRUCT`. Example:

```cpp
namespace {
NERVE_IMPL(MyClass, Idle);
NERVE_IMPL(MyClass, Move);
NERVES_MAKE_NOSTRUCT(MyClass, Idle, Move);
}  // namespace
```

**Constructor pattern**: single-expression constructors that only delegate to a base class are written on one line:

```cpp
MyClass::MyClass(const char* name) : al::LiveActor(name) {}
```

### 4. Check everything

After the first pass, run:

```
tools/check
```

Then cross-reference `file_list.yml` to confirm no functions were missed. Every function in the class should have some matching state before proceeding.

### 5. Fix non-matching functions

For each non-matching function, you have **up to 3 attempts**: rebuild and run `tools/check --no-pager <func>` after each change. If it still doesn't match after 3 tries, add a comment above the function:

```cpp
// NON_MATCHING: <explanation of what's wrong>
```

### 6. Final cleanup and verification

- Run `tools/check-format.py` and fix every formatting issue it reports.
- Run `tools/check` one final time to confirm all statuses.
- Use sead math/vector inlines wherever possible; write it as a programmer would.
- **Always verify your contribution is correct** before considering a class done: check that statuses in `file_list.yml` are accurate, that no functions were skipped, and that the build is clean.

## Code Style (summary)

Full details in `Contributing.md`. Key rules:

- C++17. 4-space indent. Max 100 characters per line.
- `#pragma once` for header guards.
- `nullptr` not `NULL` or `0`.
- Use `auto` only when type is obvious, too long, or doesn't matter.
- Compare integers against zero explicitly: `if (value == 0)`, not `if (!value)`. (Booleans are exempt.)
- Includes: angle brackets `<>` for system/sead/library headers; quotes `""` for al/rs/game headers. Three blocks separated by blank lines: `<>` first, then `al`/`Library`, then game-local.
- Type names and compile-time constants: `UpperCamelCase`.
- Function names: `camelCase`.
- Local variables and parameters: `lowercase_snake_case`.
- Class member variables: `mCamelCase`. Static members: `sCamelCase`. Globals: `gCamelCase`.
- Class member order: `public` → `protected` → `private`; constructor/destructor/operators/other functions, then non-static variables, statics before non-statics.
- Use `override` not `virtual` when overriding. Mark `const` where applicable. No `this->` unless necessary.
- `= default;` for empty constructors/destructors.
- Virtual function order must match the original executable's vtable order.
