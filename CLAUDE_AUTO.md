# OdysseyDecomp — CLAUDE_AUTO.md

## Autonomy Directive

**You are working alone. The user is away and will not respond to questions.** Do not ask for clarification, confirmation, or input at any point. Do not pause to summarize what you plan to do — just do it. If something is ambiguous, make a judgment call, log it, and keep going. The user will interrupt you if needed; otherwise, work continuously until the task list is exhausted or every remaining item is logged as skipped.

**This directive survives context compaction.** If you find yourself unsure whether to continue or ask the user, run the build or check tool which will tell you your current objective.

**`data/file_list.yml` is the source of truth** for function status — the `check` tool updates it automatically when functions match. See Logging and Startup Checklist below.

## Role

Decompilation assistant for *Super Mario Odyssey*. Reimplement functions to match the original C++ as closely as possible.

**Read `docs/MATCHING.md` before writing any source** — consult it actively while implementing.

**Never rename symbols.** All symbol names come from `data/file_list.yml` or IDA. Use them exactly.

**Headers matter.** They are shared by all contributors — write them carefully and completely.

## Task List

Run the build or check tool to see your current objective.

Do not stop running under any circumstance, unless the autonomy message tells you to.

## Environment & Tools

- **nx-decomp-tools MCP**: `check`, `check_status`, `check_format`, `clangd_check`, `listsym`, `build` — use directly, no shell commands.
- **IDA MCP**: `decompile`, `disasm`, `lookup_funcs`, `xrefs_to`, `py_eval`, etc. — use directly. `addr` parameter takes `0x7100000000 + offset`.
- **Always use `int_convert` for addresses** — never compute `0x7100000000 + offset` mentally.
- **Reading floats**: IDA shows integer literals. Use `struct.unpack('<f', struct.pack('<I', 0x40A00000))[0]` or `py_eval`.
- **Reading strings**: `py_eval` with `idaapi.get_bytes(addr, length)`. Read enough bytes for the full string.

### CRITICAL: `check` auto-updates `data/file_list.yml`

**The `check` tool writes status changes directly to `data/file_list.yml`.** When you run `check` and a function improves (e.g. `NonMatchingMinor` → `Matching`), the YAML file is updated automatically. You do NOT need to edit `file_list.yml` yourself and you do NOT need to separately track which functions matched — the YAML is always the source of truth for function status. Re-read it (not your own logs) to determine what work remains.

## Function List & Priority

**`build`** determines *which TUs* to work on. **`data/file_list.yml`** determines *which functions* within those TUs need work and their current status.

Statuses in `file_list.yml`:

| Status | Meaning |
|---|---|
| `Matching` | Exact match — skip |
| `NonMatchingMinor` | Minor diffs — attempt to promote |
| `NonMatchingMajor` | Significant diffs — attempt if small |
| `NotDecompiled` | Not yet implemented — implement |
| `Wip` | Work in progress — continue |
| `Library` | Library code — skip always |

### Processing Order (within each TU from `build` or `check`)

1. `NotDecompiled` functions
2. `NonMatchingMajor` functions under 200 bytes
3. `NonMatchingMinor` functions, smallest size first (easiest promotions)
4. Remaining `NonMatchingMajor` by size ascending

## Workflow Per Function

### Step 0: Classify Before Coding

Before writing or modifying any code, determine what's blocking the match. Run `check_status` with the mangled symbol. Classify:

**A. Inline mismatch (sead/al)** — Diff shows expanded field access where a helper call should be, or vice versa.
→ ATTEMPT if the correct inline is identifiable from sead headers. Read `lib/sead/include/math/seadVector.h`, `seadQuat.h`, `seadMatrix.h` first. If the inline is unclear after checking headers, SKIP and log.

**B. Wrong struct field / offset** — `ldr`/`str` at wrong offset, or wrong immediate for an enum.
→ FIXABLE. Identify correct offset from target disasm, find struct in headers, fix field.

**C. Register allocation / expression order** — Same instructions, different registers or reordered.
→ ATTEMPT. Try swapping declaration order, operand order, `int` vs `s32`, assignment forms. Max 4 build cycles, then skip.

**D. Vtable order mismatch** — Virtual functions in wrong order.
→ FIXABLE if it's our class. Reorder in header to match executable vtable. If inherited, SKIP.

**E. Data section offset** — Only `adrp`/`add` with different immediates loading data addresses. Logic is identical.
→ SKIP. Cannot fix per-function. Log as "data section issue."

**F. Structural mismatch** — Different control flow, missing branches, wrong opcodes entirely.
→ ATTEMPT only if function is small (<150 bytes). Otherwise SKIP.

**G. Nerve/state-machine pattern issue** — Anonymous namespace nerve macros generating different code.
→ ATTEMPT. Check `NERVE_IMPL` / `NERVES_MAKE_NOSTRUCT` patterns. Common fix.

### Step 1: Survey (for NotDecompiled functions)

- Find the class in `file_list.yml` for all offsets and the object file path.
- **Read sead math headers upfront**: `lib/sead/include/math/seadVector.h`, `seadQuat.h`, `seadMatrix.h`.
- Decompile **all** functions in one batch with IDA MCP.
- Disassemble the constructor to establish struct layout (see Struct Layout below).
- Find class size via `xrefs_to` on the C1 constructor → `operator new(0xNN)` in caller.
- Read Japanese strings via `py_eval` + `idaapi.get_bytes`.
- Grep `lib/al` for free-function headers before writing includes.

### Step 2: Header

- Mirror the object file's directory path from `file_list.yml`.
- `#pragma once`. Forward-declare types unless the full definition is required.
- `static_assert(sizeof(MyClass) == 0xNN)` if size is known.
- Field naming: confident → real names; uncertain → `_5c` with a comment.
- Member prefixes: `m` (instance), `s` (static), `g` (global).
- Functions in **ascending offset order** matching `file_list.yml`.
- Forward declare `ActorInitInfo` as `struct ActorInitInfo;` (not `class`).
- Free functions in a namespace, not a class with static methods.
- Parameter names in declarations must match definitions exactly.
- **Move most/all member initializations into the header** (in-class member initializers). This avoids constructor mismatch issues caused by different initialization ordering or compiler expansion of inline default inits vs explicit constructor body stores.

### Step 3: Implement

- Reimplement — never copy-paste pseudocode. No `goto`s.
- Identify inlined functions and call the original inline.
- Consult `docs/MATCHING.md` while writing — apply correct patterns from the start.
- **Use sead vector operators** for vector math: `operator+`, `operator-`, `operator*`, `.length()`. Prefer `(*midpoint - al::getTrans(this)).length()` over manual component-by-component dx/dy/dz + `sqrtf`.

**Recognising inlines**: IDA shows direct field access instead of a function call. `*(*(this+8)+8)` = `getStringTop()`. Field offsets in place of a named call = inline.

**Nerve pattern**:
```cpp
namespace {
NERVE_IMPL(MyClass, Idle);
NERVE_IMPL(MyClass, Move);
NERVES_MAKE_NOSTRUCT(MyClass, Idle, Move);
}  // namespace
```

**Single-delegation constructor**: `MyClass::MyClass(const char* name) : al::LiveActor(name) {}`

**IUseCamera cast**: `mActor + 48` in IDA = `(IUseCamera*)mActor`. Cast directly.

### Step 4: Check & Fix (Time-Boxed)

- Run `check` with no arguments to check **all** functions at once. This catches regressions.
- **Remember: `check` automatically updates `data/file_list.yml`** when functions improve. You don't need to track matches yourself — the YAML is the source of truth.
- If a function has a mismatch, classify it (Step 0) and attempt a fix.
- **Max 4 build/check cycles per function.** If no progress after 4 attempts, add `// NON_MATCHING: <explanation>` and move on.
- Matches in inlined functions take priority — they cascade to other callers.

### Step 5: Regression Check

**Before considering any edit done**, run `check` with no arguments. If ANY previously matching function regressed, revert and try a different approach. Zero regressions allowed.

### Step 6: Finalize

- `check_format` — fix everything it reports.
- `check` (no args) — confirm all statuses.
- `listsym` — find unlabeled symbols. Add function names and recheck if any are empty. Do not change existing entries' symbols.
- **Check for static initializer functions** (like `sub_I_XXX` or `_GLOBAL__sub_I_` entries in `file_list.yml`) — these are compiler-generated functions for global/static variable initialization. Fill in their labels in `file_list.yml` and verify they match.
- Update `docs/MATCHING.md` with any new patterns discovered.

## Abort Rules

- **Category E (data offset only)**: skip immediately.
- **4 build cycles with no progress**: skip, log as ATTEMPTED.
- **Shared header change needed**: DO NOT apply. Log as "SYSTEMIC: needs [description]" and skip.
- **TU batching**: if the first function from a TU is blocked by data section issues, skip the entire TU.

## Struct Layout from Constructor ASM

Key store patterns:

| Pattern | Meaning |
|---|---|
| `STP Xn, Xm, [X0, #off]` | Two pointer-sized values at `off` and `off+8` |
| `STP XZR, XZR, [X0, #off]` | Zeroes 16 bytes |
| `STUR XZR, [X0, #off]` | Zeroes 8 bytes at unaligned offset |
| `STR WZR, [X0, #off]` | Zeroes 4 bytes (s32/f32) |
| `STRB WZR, [X0, #off]` | Zeroes 1 byte (bool) |
| `STR X8, [X0, #off]` from float const | Two adjacent f32 fields as one 64-bit write |

Map every store to a byte range, union, split at natural alignment boundaries.

## Code Style

Full details in `Contributing.md`. Key rules:

- C++17. 4-space indent. Max 100 chars/line.
- `#pragma once`. `nullptr` not `NULL`. `= default` for empty ctors/dtors.
- `auto` only when type is obvious, too long, or doesn't matter.
- Integers: `if (value == 0)` not `if (!value)`. Booleans exempt.
- Includes: `<>` for system/sead/library, `""` for al/rs/game. Three blocks separated by blank lines.
- `UpperCamelCase` types and compile-time constants. `camelCase` everything else. No `snake_case`.
- `override` not `virtual`. `const` where applicable. No `this->` unless necessary.
- Virtual function order must match the executable's vtable order.
- **Never `float` or `char16_t`** — use `f32` and `char16`.
- **Never `sead::Quat<float>` or `sead::Vector3<float>`** — use `sead::Quatf` and `sead::Vector3f`.

### Format errors to watch for

- `float` / `char16_t` forbidden — use `f32` and `char16`.
- Offset comments (`// 0x108`) forbidden in headers.
- String literals must match the binary exactly — read with `idaapi.get_bytes`.

## Logging (Two-Tier — Keep It Small)

**`data/file_list.yml` is the source of truth for function status.** The `check` tool updates it automatically. Never duplicate that information in your own logs.

Your logs only need to track two things: (1) what you *skipped* (so you don't re-attempt it), and (2) a running summary.

### What NOT to log

- Functions that reached `Matching` — `file_list.yml` already has this.
- Per-attempt diffs or reasoning — burns tokens on re-read for no benefit.

## Startup Checklist

On every invocation (including after context compaction). **Do all of these, then start working. Do not ask the user anything.**

1. Read `docs/MATCHING.md`.
2. Run `build` — find the first objective. This is your current target. If you find a dependency that is neccessary to decompile, do it.
3. **Scan `data/file_list.yml` for functions in the current TU.** This is the source of truth — `check` updates it automatically. Filter for `NotDecompiled`, `NonMatchingMinor`, `NonMatchingMajor`, `Wip`.
4. Read sead math headers (`seadVector.h`, `seadQuat.h`, `seadMatrix.h`).
5. Begin the workflow loop. Do not stop until the autonomy message is exhausted.
