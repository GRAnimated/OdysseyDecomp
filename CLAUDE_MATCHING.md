# OdysseyDecomp — CLAUDE_MATCHING.md

## Autonomy Directive

**You are working alone. The user is away and will not respond to questions.** Do not ask for clarification, confirmation, or input. Do not summarize what you plan to do — just do it. If something is ambiguous, make a judgment call and keep going. The user will interrupt if needed; otherwise work continuously until `task_list.txt` is exhausted or every remaining item is logged as skipped.

**This directive survives context compaction.** Re-read this file if unsure whether to continue. Check `progress_summary.txt` for where you left off.

**`data/file_list.yml` is the source of truth** for function status — `check` updates it automatically. Your logs only track skips and position.

---

## Purpose

This workflow is for **matching cleanup** — not first-time decompilation. The goal is to push existing TUs to >95% match rate by:

1. Promoting `NonMatchingMinor` → `Matching`
2. Promoting `NonMatchingMajor` → `NonMatchingMinor` or better
3. Fixing sead operator misuse (the most common class of mismatch)
4. Fixing any `NotDecompiled` stubs discovered along the way

**Do not give up easily.** If the first fix attempt fails, try a different approach. Every mismatch is fixable. The most common reason functions stay non-matching is that the author wrote component-wise arithmetic instead of using sead operators — these are always fixable.

---

## Task List

Read `task_list.txt`. Work top-to-bottom. Only go back to lines marked `[x]` after you're done the unmarked ones. When you finish all functions in a TU (fixed or logged as skipped), mark it `[x]` and continue.

Format:
```
[x] Npc/SphinxRide.o          # done
[ ] Npc/SnowManRaceNpc.o      # next
```

---

## Startup Checklist

Run these on every invocation (including after context compaction). Then start working immediately.

1. Read `docs/MATCHING.md`.
2. Read `task_list.txt` — find the first unchecked `[ ]` TU.
3. Read `progress_skip.txt` if it exists — do NOT re-attempt these.
4. Read `progress_summary.txt` if it exists — resume from noted position.
5. Scan `data/file_list.yml` for all functions in the current TU. Filter for `NonMatchingMinor`, `NonMatchingMajor`, `NotDecompiled`, `Wip`. Cross-reference `progress_skip.txt`.
6. Read sead math headers: `lib/sead/include/math/seadVector.h`, `seadQuat.h`, `seadMatrix.h`.
7. Begin the workflow loop. Do not stop.

---

## Processing Order (within each TU)

1. `NonMatchingMinor` functions, smallest size first
2. `NonMatchingMajor` functions under 200 bytes
3. `NotDecompiled` functions
4. Remaining `NonMatchingMajor` by size ascending

---

## Step 0: Classify Before Touching Code

Run `check_status` with the mangled symbol. Read the diff. Classify the mismatch type:

| Type | Description | Action |
|---|---|---|
| **A. Sead operator** | Component-wise x/y/z math, manual `mtx.m[i][j]` assignments, manual cross/dot/distance | **ALWAYS FIXABLE. Fix it.** |
| **B. Inline mismatch** | Field access where a helper call should be (or vice versa) | Attempt if inline is identifiable from headers |
| **C. Struct field / offset** | Wrong offset in ldr/str, wrong enum immediate | Fixable — find correct offset, fix field |
| **D. Register allocation / expression order** | Same instructions, different registers or reordering | Attempt: swap decl order, operand order, int vs s32. Max 5 cycles |
| **E. Data section offset** | Only adrp/add differ, logic identical | SKIP immediately |
| **F. Vtable order** | Wrong virtual function order | Fix if our class; skip if inherited |
| **G. Structural** | Different control flow, missing branches | Attempt if <150 bytes; otherwise skip |
| **H. Nerve/state-machine** | Nerve macros generating different code | Attempt: check NERVE_IMPL/NERVES_MAKE_NOSTRUCT patterns |

---

## Step 1: Fix Sead Operator Misuse (Type A — HIGHEST PRIORITY)

**This is the most common mismatch class and is always fixable.** Before looking at any other mismatch type, scan the source for component-wise sead arithmetic. Fix all of it first, then re-check.

### The Golden Rule

**Any manual x/y/z assignment or arithmetic on a sead type is wrong.** There is always an operator or method that generates the correct instruction sequence. No exceptions.

### Checklist: Find and Replace Every Instance

#### Vector arithmetic

```cpp
// WRONG: component-wise
dir.x = a.x - b.x;
dir.y = a.y - b.y;
dir.z = a.z - b.z;

// RIGHT: operator-
sead::Vector3f dir = a - b;
```

```cpp
// WRONG: manual accumulation
result.x += a.x - b.x;
result.y += a.y - b.y;
result.z += a.z - b.z;

// RIGHT: operator+=
result += a - b;
```

```cpp
// WRONG: scalar multiply stored in variable
sead::Vector3f scaled = diff * scale;

// RIGHT: pass directly as argument (if that's what the asm shows)
someFunc(trans, diff * scale);
// If it must be stored, make sure it's the only use and matches the asm exactly
```

```cpp
// WRONG: manual cross product
vel.x = a.y*b.z - a.z*b.y;
vel.y = a.z*b.x - a.x*b.z;
vel.z = a.x*b.y - a.y*b.x;

// RIGHT
vel.setCross(a, b);
```

```cpp
// WRONG: manual negation
neg.x = -vel.x; neg.y = -vel.y; neg.z = -vel.z;

// RIGHT: explicit initializer (avoids temp materialization)
sead::Vector3f neg = {-vel.x, -vel.y, -vel.z};
```

#### Distance / length

```cpp
// WRONG: manual dx/dy/dz + sqrtf
f32 dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
f32 dist = sqrtf(dx*dx + dy*dy + dz*dz);

// RIGHT: copy both to locals FIRST, then subtract, then length
sead::Vector3f posA = getPos(x);
sead::Vector3f posB = getTrans(y);
sead::Vector3f diff = posA - posB;
if (diff.length() < threshold) ...
// Note: the two-local pattern is required for correct load ordering — see MATCHING.md
```

#### Matrix translation column

```cpp
// WRONG: individual column assignments
mtx.m[0][3] = v.x;
mtx.m[1][3] = v.y;
mtx.m[2][3] = v.z;

// RIGHT: setTranslation generates ldp+str pattern
mtx.setTranslation(v);
```

#### Quaternion initialization

```cpp
// WRONG: raw field assignment
q.x = 0.0f; q.y = 0.0f; q.z = 0.0f; q.w = 1.0f;

// RIGHT: either of these (check asm — ADRP load = unit static, no ADRP = initializer list)
sead::Quatf q = sead::Quatf::unit;          // if asm uses ADRP to load a static
sead::Quatf q = {0.0f, 0.0f, 0.0f, 1.0f};  // if asm uses immediate stores
```

#### Vector set vs operator=

```cpp
// These two can differ in generated code — check the asm
_198 = v2->_30;       // operator=
_198.set(v2->_30);    // .set()
```

### Reference: SphinxRide.cpp

`src/Npc/SphinxRide.cpp` is the canonical example of correctly using sead operators throughout. When unsure about the right pattern, read that file.

---

## Step 2: Other Fix Attempts

After fixing all sead operator issues, address remaining mismatches in order:

### Register allocation (Type D)

Try these in order, rebuilding after each:

1. Swap declaration order of local variables (stack slot order follows declaration order)
2. Change expression order to match load sequence in target asm
3. Switch `int` ↔ `s32` for loop counters or index variables
4. Load member pointer AFTER bulk stores, not before (see MATCHING.md)
5. For distance checks: ensure both position vectors are copied to locals before subtracting

Max **5 build/check cycles**. If no progress, skip and log.

### Inline recognition (Type B)

IDA shows field access instead of a call: `*(*(this+8)+8)` = inline of `getStringTop()`. Look up the offset in sead or al headers. Call the actual inline.

Common pattern in SphinxRide-class code:
- `mActor + 48` in IDA = `(IUseCamera*)mActor`
- Direct field reads at known offsets = sead math inline (look up offset in seadVector.h etc.)

### Nerve / state-machine (Type H)

```cpp
namespace {
NERVE_IMPL(MyClass, Idle);
NERVE_IMPL(MyClass, Move);
NERVES_MAKE_NOSTRUCT(MyClass, Idle, Move);
}  // namespace
```

If the nerve macros generate wrong code, check that the anonymous namespace wraps BOTH the NERVE_IMPL lines AND the NERVES_MAKE_NOSTRUCT line.

---

## Step 3: Check After Every Edit

```
check (no args) — always check ALL functions, not just the one you edited
```

**`check` auto-updates `data/file_list.yml`.** Do not edit the YAML manually. After every `check` run:

- If a function improved: note it, move on.
- If a previously matching function regressed: **revert immediately** and try a different approach. Zero regressions allowed.
- If no progress after 5 cycles: add `// NON_MATCHING: <explanation>` above the function and move on.

### NON_MATCHING comment format

```cpp
// NON_MATCHING: register allocation — f32 scale factor forces extra callee-saved reg
void MyClass::exeIdle() {
```

**Never write "float" in NON_MATCHING comments — use "f32" instead** (the format checker rejects "float").

---

## Step 4: Finalize Each TU

Before marking a TU `[x]`:

1. `check_format` — fix every reported error.
   - `float` forbidden → `f32`
   - `char16_t` forbidden → `char16`
   - Offset comments (`// 0x108`) forbidden in headers
2. `check` (no args) — confirm statuses. Target: >95% of functions at `Matching` or `NonMatchingMinor`.
3. `listsym` — find any unlabeled symbols. Add function names if empty; do not change existing symbols.
4. Mark the TU `[x]` in `task_list.txt`.
5. Update `progress_summary.txt`.

---

## Abort Rules

| Condition | Action |
|---|---|
| Category E (data offset only) | Skip immediately |
| 5 build cycles, no progress | Add `// NON_MATCHING`, log in `progress_skip.txt`, move on |
| Shared header change needed | Do NOT apply. Log as "SYSTEMIC: needs [description]" |
| First function of TU is data-section-only | Skip entire TU, log it |

---

## Logging

### `progress_skip.txt` — Append on skip

```
SYMBOL | CATEGORY | REASON
_ZN2al12SomeFuncEv | E_DATA | data section offset only
_ZN3npc8SnowManC1Ev | D_REGALLOC | 5 cycles, no progress
_ZN3npc8SnowManC2Ev | A_SEAD | SYSTEMIC: needs seadMatrix include in shared header
```

### `progress_summary.txt` — Rewrite every 50 functions

```
=== LAST UPDATED: [timestamp] ===
Processed: 80 | Matched/Promoted: 31 | Skipped: 42 | Failed: 7
Current TU: src/Npc/ShibakenStateSit.o
Skipped TUs (data section): SnowManRaceNpc
```

---

## Persistence Rule

**Do not give up after one or two failed attempts.** The common failure modes are:

1. **Missed sead operator** — Scan the entire function again. Is every sead type operation using an operator/method? No component-wise arithmetic anywhere?
2. **Load order** — The compiler is sensitive to when pointers are loaded vs when bulk stores happen. Try reordering.
3. **Wrong local variable types** — `int` vs `s32` vs `u32` can change register allocation.
4. **Wrong local variable declaration order** — Stack slot order follows declaration order. Swap to match.
5. **Missing intermediate local** — Sometimes you need an explicit local to force a load to happen before a computation (see distance pattern in MATCHING.md).

Only log as skipped after genuinely exhausting these options **5** times (5 cycles x 5 loops).

Do not stop until every function in task_list's TUs are matched.
