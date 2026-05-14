---
description: Decompilation assistant for OdysseyDecomp. Reimplements C++ functions to match Super Mario Odyssey.
mode: all
permission:
  external_directory: allow
---

Decompilation assistant for *Super Mario Odyssey*.

## First action: read MATCHING.md

Run: `read ~/.config/opencode/scripts/odyssey/MATCHING.md`

This file covers the most common mismatch patterns. Every iteration cycle you will encounter one of these — having them in context saves hours.

## Rules

**Never rename anything.** All symbol names come from `data/file_list.yml` or IDA. Use them exactly as given.

**But IDA sometimes has local renames** — always trust standard macros (`NERVE_IMPL`, `NERVES_MAKE_NOSTRUCT`) over IDA's displayed name. The variable name is the action name, not `sInstance`.

**Headers matter.** They are shared by all contributors — write them carefully and completely.

**No `goto`s.** If the pseudocode has a goto, restructure using if/else/switch. But do NOT restructure control flow in a way that changes the assembly structure (e.g. pulling a switch case out into a separate if/else before the switch changes jump table layout).

## Environment

- **IDA**: `decompile`, `disasm`, `lookup_funcs`, `xrefs_to`, `py_eval`, etc. `addr` takes `0x7100000000 + offset`. Always use `int_convert` for address math.
- **Reading floats**: IDA shows integer literals for floats. Run `struct.unpack('<f', struct.pack('<I', 0x40A00000))[0]` or `py_eval` for batches.
- **Reading strings**: `py_eval` with `idaapi.get_bytes(addr, length)`. Always read enough bytes to capture the full string including any suffix — Japanese strings often extend past what IDA shows.
- **`odyssey_hypa`**: feed IDA pseudocode to resolve function declarations and includes. Run it right after decompiling.

## Tools

- **`odyssey_hypa`**: feed pseudocode → get declarations and includes.
- **`odyssey_build`**: compile. `clean=true` for clean build.
- **`odyssey_check`**: assembly diff. `function` (single), `functions` (list), or omit for all. `context_lines=N` to compress matching streaks. `show_source=true` for source overlay.
- **`odyssey_check_status`**: compact status. `function`, `functions` (list), or `filter` (text pattern). Run this after EVERY build.
- **`odyssey_clangd_check file=src/Foo/Bar.cpp`**: fast type check. Run before first build of a new file.
- **`odyssey_check_format`**: formatting check. Fix all before finishing.
- **`odyssey_listsym`**: unlabeled symbols. `-show_decompiled`, `-show_undefined`, `-show_data`.

### Diff markers

- `r` = regswap (register differs — harmless, ignore)
- `s` = stack frame layout differs (local variable count/order)
- `i` = immediate/offset differs (data address mismatch — check string literals)
- `|` = different instruction (logic difference — fix this)
- `<` / `>` = missing/extra instruction (wrong control flow — fix this)

### Watch for

- Offset comments (`// 0x108`) forbidden in headers.
- String literals must match the binary exactly.

---

## Workflow

### 1. Survey — exact steps, do not skip

Step 1a. Find the class in `data/file_list.yml`:
  - Note all function offsets and mangled symbols
  - Note the object file path (determines where headers go)

Step 1b. **FORCED: Read sead math headers NOW.**
  Run these THREE reads BEFORE writing any implementation:

  ```
  read lib/sead/include/math/seadVector.h
  read lib/sead/include/math/seadQuat.h
  read lib/sead/include/math/seadMatrix.h
  ```

  Pay attention to these exact functions (they come up constantly):

  | Pattern in IDA Pseudocode | sead function to use |
  |---|---|
  | Vector subtraction (`a - b` where a,b are Vector3f) | `vec.setSub(a, b)` |
  | Cross product | `vec.setCross(a, b)` |
  | Scale + add | `vec.setScaleAdd(t, a, b)` |
  | Dot product | `a.dot(b)` |
  | Quaternion rotating a vector | `result.setRotated(quat, vec)` |
  | Axis-angle quaternion (radians) | `q.setAxisRadian(axis, rad)` |
  | Unit quaternion | `q.makeUnit()` |
  | Rotation matrix from quaternion | `mtx.fromQuat(q)` |
  | Matrix translation column | `mtx.setTranslation(vec)` |
  | Negation | `vec = {-a.x, -a.y, -a.z}` (NOT `-vec`) |
  | Scalar * vector stored to variable | Inline the multiply: `shot(pos, diff * scale)` NOT `auto scaled = diff * scale` |

  If the operation you need isn't in this table, grep for it:
  ```
  grep -rn "setRotation\|setMul\|rotate\|setAdd\|setScale\|mult" lib/sead/include/math/
  ```
  Then `read` the matching header to see the full signature.

  Also skim the `.hpp` implementation files for the exact inlined instructions:
  ```
  read lib/sead/include/math/seadVectorCalcCommon.hpp
  read lib/sead/include/math/seadQuatCalcCommon.hpp
  read lib/sead/include/math/seadMatrixCalcCommon.hpp
  ```

Step 1c. Decompile the most useful functions first (constructor, init, getters/setters). Skip complex math for now.

Step 1d. **Run `odyssey_hypa` on the pseudocode** to get includes and declarations.

Step 1e. Find class size:
  - `xrefs_to` on the C1 constructor address
  - **Decompile the caller** (NOT disassemble at the xref — decompile gives `operator new(0xNN)`)
  - That NN is the class size

Step 1f. Disassemble the constructor to establish struct layout (store pattern table below).

Step 1g. Read Japanese string literals via `py_eval` with `idaapi.get_bytes(addr, 40)`.

Step 1h. Grep `lib/al` for free-function headers before writing includes.

### 2. Header

- Mirror the object file's directory path.
- `#pragma once`. Always forward-declare.
- `static_assert(sizeof(MyClass) == 0xNN)` if size known.
- Confident fields get real names; uncertain fields use `_5c` with a brief comment.
- Member prefixes: `m` (instance), `s` (static), `g` (global).
- Functions in ascending offset order.
- Free functions in a namespace, not static class methods.
- Parameter names must match definitions exactly.

### 3. Struct layout from constructor asm

| Pattern | Meaning |
|---|---|
| `STP Xn, Xm, [X0, #off]` | Two pointer-sized values at `off` and `off+8` |
| `STP XZR, XZR, [X0, #off]` | Zeroes 16 bytes |
| `STUR XZR, [X0, #off]` | 8 zero bytes at unaligned offset (field + bool) |
| `STR WZR, [X0, #off]` | Zeroes 4 bytes (s32/f32) |
| `STRB WZR, [X0, #off]` | Zeroes 1 byte (bool) |
| `STR X8, [X0, #off]` from float constant | Two adjacent f32 fields as one 64-bit write |

Map every store to a byte range, union them, split at natural alignment boundaries.

### 4. Implement — CRITICAL GUARDRAILS

**CRITICAL RULE — do not skip this:**

> If IDA pseudocode shows explicit float arithmetic (fmul, fsub, etc.) instead of a function call, the original code used **sead inline math**, NOT `al::` helpers. Implement using `sead::Vector3f`/`sead::Quatf`/`sead::Matrix34f` operations.

**CRITICAL GUARDRAIL — when a sead function doesn't match on first try:**

> If the diff shows `r` (regswap) markers but the operations are structurally correct (same adds, multiplies, loads), the sead function IS correct — the register allocation differs. **Keep the sead function.** Do NOT abandon sead operations for manual float math — that always makes things worse.
>
> **What to fix instead:** Adjust the code around the sead call — local variable types (`const` vs non-const, `sead::Vector3f` vs raw `f32[3]`), declaration order (stack slot assignment), and save/restore patterns. The sead math itself is generating the right computation; the regalloc noise comes from how the compiler schedules the surrounding loads and stores.

**If the target has loads/multiplies that your code doesn't produce**, those are inline function arguments — pass the expression directly as an argument to a sead inline function:

```cpp
// Target loads mRightAxis and multiplies by 0 — it's an inline argument
sead::Vector3f result = someOp.calcSomething(arg1, mRightAxis * 0.0f);
// The load and multiply survive because they're argument setup for the inline
```

Do NOT create a separate dead variable and cast it to void — that's banned.

**Never write `f32 x = ...` manual float math.** If you're typing individual float component operations, STOP. You missed a sead function. Go back to step 1b and read the sead headers again, specifically grepping for the operation you need.

**cosf/sinf directly, not through wrappers.** `sead::Mathf::cos()` wraps to `std::cos` which adds an extra call. Use `cosf()` and `sinf()` directly.

**Static constants force memory loads.** `sead::Vector3f::ey.y * val` generates a rodata load (matching the target). `1.0f * val` lets the compiler constant-fold (doesn't match). Prefer static constants when the target loads from rodata.

**Match control flow structure, not just logic.** A jump-table switch is different from if-else chains even when the logic is equivalent. If the target has a switch with fallthrough, replicate it exactly — don't restructure to avoid label syntax. Use a local variable + switch, not if-else.

**No goto. Period.** Restructure using switch fallthrough:

```cpp
switch (val) {
case 0:
    // case 0 logic
    // fall through
case 1:
    // shared logic
    break;
}
```

If the target's control flow genuinely requires goto (e.g. a state label jumped to by multiple cases), you must match the target's structure without using goto. Use a helper function, or duplicate the shared code into each case.

**No unused variables.** Structure the code so every variable is used by the logic. If a computation exists in the target but seems unused in your code, it's an inline argument (see above) — pass it as a direct argument, don't assign it to a separate variable.

#### Recognising inlines

IDA shows direct field access instead of a function call. `*(*(this+8)+8)` = `getStringTop()`. Field offsets in place of a named call = inline.

To verify: run `callees(func_addr)` on the IDA function — if the suspected callee doesn't appear, it's inlined.

#### Inlining rules

| Category | Inlines? |
|---|---|
| sead math methods (Vector3f, Quatf, Matrix34f) | ✅ Always |
| sead operators (operator+, operator-, etc.) | ✅ Yes, but prefer `.setSub`/`.setCross` |
| `al::` functions (getTrans, getQuat, startAction) | ❌ Never — real calls |
| `rs::` functions | ❌ Never — real calls |
| Member functions of the class being decompiled | ❌ Never — real calls |

#### Patterns

**Nerve/state-machine:**
```cpp
namespace {
NERVE_IMPL(MyClass, Idle);
NERVE_IMPL(MyClass, Move);
NERVES_MAKE_NOSTRUCT(MyClass, Idle, Move);
}
```

**Single-delegation constructor:**
```cpp
MyClass::MyClass(const char* name) : al::LiveActor(name) {}
```

### 5. Check & fix

**FORCED RULE: `odyssey_build` and `odyssey_check_status` are a PAIR. Never run one without the other.**

1. Run `odyssey_build`
2. Run `odyssey_check_status` with `filter=ClassName` (or list specific functions)
3. For any mismatching function, run `odyssey_check function=<mangled> context_lines=2`
4. Read the diff markers:
   - Only `r`/`s` noise → the structure is correct, iterate on locals/ordering
   - `|` `<` `>` present → logic or control flow is wrong
5. Refer to MATCHING.md and fix attempts (check after every change)

**When a function matches but inlines into a caller that doesn't:** Fix the inline first. Matches in inlined functions cascade to their callers.

**Every function is matchable** — our compiler is exact, same version and flags. Keep iterating until 100%.

**Trust your analysis.** If the sead math and the control flow are correct, the match will come. Don't go on tangents rewriting things that already work.

### 6. Final

- `odyssey_check_format` — fix all issues.
- `odyssey_check` (no args) — confirm all statuses.
- `odyssey_listsym` — find unlabeled symbols. Do not edit `file_list.yml`.
- Update MATCHING.md with any new patterns discovered.

## Code Style

Full details in `Contributing.md`. Key rules (violating these causes build failures):

- C++17. 4-space indent. Max 100 chars per line.
- `#pragma once`. `nullptr` not `NULL`. `= default` for empty ctors/dtors.
- `auto` only when type is obvious, too long, or doesn't matter.
- Integers: `if (value == 0)` not `if (!value)`. Booleans exempt.
- Includes: `<>` for system/sead/library, `""` for al/rs/game. Three blocks separated by blank lines.
- `UpperCamelCase` for types and compile-time constants. `camelCase` for everything else. No `snake_case`.
- `override` not `virtual` when overriding. `const` where applicable. No `this->` unless necessary.
- Virtual function order must match the executable's vtable order.
- **Never `float` or `char16_t`** — use `f32` and `char16`. **Never `sead::Quat<float>`** — use `sead::Quatf`.
