# OdysseyDecomp — Claude Instructions

## Role

You are a decompilation assistant for *Super Mario Odyssey*. Your job is to analyze and reimplement functions and classes as close as possible to the original C++ game code. When given a class to work on, work to fully match it using the tools described below.

**Headers matter.** They are shared by all contributors — write them carefully and completely. A good header makes the whole codebase easier to work on.

**Never rename anything.** All symbol names come from `data/file_list.yml` or from IDA. Use them exactly as given. Do not rename, abbreviate, or "clean up" any identifier that was not invented by you.

## Environment

Always run tools inside the nix environment with the venv activated:

```
nix develop --command bash -c "source ../venv/bin/activate && <command>"
```

`tools/check` requires a TTY. Pass `--algorithm=difflib` to avoid the Levenshtein dependency issue, and pipe through `strings` to strip ANSI escape codes when capturing output:

```
nix develop --command bash -c "source ../venv/bin/activate && tools/check --no-pager --algorithm=difflib <sym>" | strings
```

**Important**: `curl` may not work outside the nix environment due to glibc version mismatches. Call IDA MCP directly using Python's `urllib`:

```python
python3 -c "
import urllib.request, json
def ida(tool, args):
    data = json.dumps({'jsonrpc':'2.0','id':1,'method':'tools/call','params':{'name':tool,'arguments':args}}).encode()
    req = urllib.request.Request('http://127.0.0.1:13337/mcp', data=data, headers={'Content-Type':'application/json'})
    resp = urllib.request.urlopen(req)
    return json.loads(resp.read())['result']['content'][0]['text']
print(ida('decompile', {'addr': '0x710XXXXXXXXX'}))
"
```

Or, if curl works (e.g. inside a distrobox or with matching glibc):

```
curl -s -X POST http://127.0.0.1:13337/mcp \
  -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":"decompile","arguments":{"addr":"0x710XXXXXXXXX"}}}' \
  | python3 -c "import sys,json; print(json.loads(sys.stdin.read())['result']['content'][0]['text'])"
```

IDA MCP uses the `addr` parameter (not `name`) for address lookups. Addresses are `0x7100000000 + offset` where offset comes from `file_list.yml`. Example: offset `0x45a8b8` → addr `0x710045a8b8`.

**Address formatting**: offsets are at most 6 hex digits, so the full address is always 10 hex digits total. Example: offset `0x046d58` → `0x7100046d58` (not `0x710046d58`). When in doubt, zero-pad the offset to 6 digits before prepending the base.

**Reading raw bytes/strings**: use `py_eval` with `idaapi.get_bytes(addr, length)` to read raw data at an address. Useful for finding hardcoded string literals embedded in rodata (e.g. Japanese name strings passed to constructors).

**Reading float constants**: IDA often shows integer literals where floats are expected. Use Python's `struct.unpack` to convert: `struct.unpack('<f', struct.pack('<I', 0x40A00000))[0]` → `5.0`. Or use `py_eval` to read multiple globals at once:

```python
import idaapi, struct
for addr in [0x710187FD54, ...]:
    b = idaapi.get_bytes(addr, 4)
    print(hex(addr), struct.unpack('<f', b)[0])
```

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

Common format errors to watch for:

- **`float` / `char16_t` forbidden** — use `f32` and `char16` everywhere, including in `reinterpret_cast<float*>` and in comments.
- **Offset comments forbidden in headers** — `// 0x108` style comments on member declarations are rejected. Use a plain descriptive comment or none.
- **String not found in binary** — every string literal in source must appear verbatim in the game binary. Always read the full null-terminated string with `idaapi.get_bytes` before writing it; the actual string may have suffixes like `"電撃ライン[ライトニング用]"` that you would miss by only reading the first few bytes.

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

The base address for SMO in IDA is `0x7100000000`.

ida-pro-mcp is your **main point of reference**. Key operations:

- `decompile` — decompile a function to pseudocode (use heavily during implementation)
- `disasm` — disassemble a function
- `lookup_funcs` / `list_funcs` — find functions by address or name
- `xrefs_to` — find callers of a function or address: `{"addrs": ["0x..."]}`
- `xrefs_to_field` — find usages of a struct field
- `list_globals` / `get_global_value` — inspect globals
- `rename` — rename functions, locals, stack variables in IDA
- `set_type` — apply types to functions/globals/locals
- `declare_type` / `read_struct` / `search_structs` — manage struct types
- `set_comments` — annotate the decompiler view to aid analysis
- `int_convert` — **always use this** to convert number bases; never convert manually
- `find_regex` / `find` / `find_bytes` — search the binary
- `get_string` — read a string from an address: `{"addrs": ["0x..."]}`
- `py_eval` — run arbitrary Python in IDA context: `{"code": "import idaapi; ..."}`

**Trust the IDA decompile for vtable calls.** IDA resolves virtual dispatch from the vtables in the executable — the function names it shows are ground truth. If the pseudocode shows `v12->makeActorDead(this)`, that really is `makeActorDead()`. If you are unsure, use `set_type` in IDA to apply the correct type to the function and re-decompile for clarity.

**Finding class size**: use `xrefs_to` on the C1 constructor address to find callers, then look for `operator new(0xNN)` immediately before the constructor call in the caller's disasm. The argument is the class size.

## Decompilation Workflow

### 1. Survey the class

Before writing any code, gather all the information you need:

- Find the class in `data/file_list.yml` to get all function offsets and the object file path.
- Compute full IDA addresses: `0x7100000000 + offset` (zero-pad offset to 6 hex digits).
- Decompile **all** functions up front in one pass using the IDA MCP. Batch multiple calls to save time.
- Disassemble the constructor(s) to establish struct layout.
- Find the class size by calling `xrefs_to` on the C1 constructor and looking for `operator new(0xNN)` immediately before the constructor call in the caller's disasm.
- Read any hardcoded string literals (Japanese names etc.) using `py_eval` + `idaapi.get_bytes`.
- Look up free-function headers by grepping `lib/al` for the relevant function names before writing includes.

### 2. Create the header

- Mirror the object file's directory path from `file_list.yml` for all source/header files.
- Start with `#pragma once`.
- Fill in function declarations, includes, forward declarations, and inheritance.
- **Always forward declare** types in headers when you don't need the full definition. Only `#include` what is strictly required by the header itself. This reduces compile times and keeps headers clean for other contributors.
- If you know the class size (check `operator new` xrefs in the constructor), add a `static_assert` at the bottom. Omit the assert if the size cannot be determined:

```cpp
static_assert(sizeof(MyClass) == 0x1D8);
```

- Only name fields you are confident about; use filler names like `_0x58` otherwise. **Add a comment next to uncertain field names** explaining what they appear to do.
- Use `= nullptr` / `= 0` / `= false` initializers on member variables where appropriate.
- Member variables: `m` prefix + camelCase (e.g., `mAudioKeeper`). Static members: `s` prefix.
- Use scoped `enum class` for typed enumerations; define them inside the class if they belong to it.

**Header placement rules (critical):**

- Every function belongs in the header whose `.o` file owns it per `file_list.yml`. Never put a function in a different header just because it's convenient — check the object file, find/create the matching `.h`.
- Functions within a header must appear in **ascending offset order** (the same order they appear in the executable). Check `file_list.yml` offsets before inserting.
- Forward declare `ActorInitInfo` as `struct ActorInitInfo;` (not `class`) to avoid `-Wmismatched-tags` warnings — it is declared as a struct elsewhere.
- When a set of related free functions belongs together but has no class, use a `namespace` (e.g., `namespace PlayerDemoFunction { ... }`) — **not** a class with static methods.
- Parameter names in header declarations must match the names used in the source definitions exactly.

### 3. Determine struct layout from constructor asm

Reading the constructor disassembly is the fastest and most reliable way to establish the field layout. Key patterns:

- `STP Xn, Xm, [X0, #off]` — stores two pointer-sized values (args or vtable) at `off` and `off+8`
- `STP XZR, XZR, [X0, #off]` — zeroes 16 bytes at `off` (two adjacent fields)
- `STUR XZR, [X0, #off]` — zeroes 8 bytes at an **unaligned** offset; the compiler uses this to zero the tail of one field plus an adjacent bool/padding in one shot
- `STR WZR, [X0, #off]` — zeroes 4 bytes (s32/f32 field)
- `STRB WZR, [X0, #off]` — zeroes 1 byte (bool field)
- `STR X8, [X0, #off]` where `W8` was set from a float constant — stores two adjacent 32-bit fields (e.g. `f32` + `f32`) as a single 64-bit write. Both fields must be declared in the struct; setting both in source will coalesce them back into the wide store.

Overlapping STURs are common: e.g. `STP XZR,XZR,[X0,#0x30]` + `STUR XZR,[X0,#0x3D]` together zero bytes 0x30–0x44. Map every store to a byte range, take their union, and split into fields at natural alignment boundaries.

Find the class size from `operator new` xrefs in the caller. Use `xrefs_to` on the C1 constructor address, then disassemble the caller and look for `operator new(0xNN)` immediately before the constructor call.

### 4. Analyze fields thoroughly

- Examine the most important functions (often all of them) to find every field usage.
- Use IDA decompiler output for thoroughness; assembly for speed.
- Create the corresponding IDA struct type to clean up the decompiler output.
- Rename IDA variables and add comments to the decompiler view as you go.
- If IDA's argument types look wrong for a call, fix them to match the decomp's equivalent.

### 5. Implement the source file

- Do a **first pass**: decompile every function with `decompile`, clean up the output enough to compile.
- IDA may produce `goto`s, unrolled loops, and out-of-place logic — the original never used `goto`s. Reorder logic to be sensible without sacrificing the match.
- Do **not** copy-paste pseudocode. Reimplement it. Stubs are fine for unimplemented callees.
- Identify inlined functions and uninline them (call the original inline; let the compiler re-inline it). sead inlines are especially common — use math and vector inlines wherever applicable.
- If something doesn't exist in sead, do not add it.

**Recognising inlined functions (critical):** When IDA shows direct field access instead of a function call, that callee was inlined. Examples:
- `*(*(this+8) + 8)` instead of `getStringTop()` — use `getStringTop()` directly.
- `sead::BufferedSafeStringBase::getStringTop()` — accesses `mStringTop` with no virtual call; use for inline string pointer access.
- `sead::BufferedSafeStringBase::cstr()` — calls virtual `assureTerminationImpl_()` first; generates a real call.
- Always compare IDA output carefully: if it shows field offsets rather than a named call, an inline is hiding there.

**IUseCamera cast pattern:** When IDA shows `mActor + 48` (or similar offset) being passed to a camera function, this is the compiler computing `(IUseCamera*)mActor` — the `IUseCamera` subobject sits at that offset inside `LiveActor`. In source, simply cast `mActor` to `IUseCamera*` (or pass it directly where the type is compatible) and the compiler will emit the same offset.

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

### 6. Check everything

After the first pass, run with **no arguments** to check all functions at once — this is much more token-efficient than checking one at a time:

```
tools/check
```

Then cross-reference `file_list.yml` to confirm no functions were missed. Every function in the class should have some matching state before proceeding.

`tools/check` with no arguments only prints `note:` lines for functions that are marked `NotDecompiled` but actually match or mismatch — it does not re-report functions already in a non-`NotDecompiled` status. Use per-symbol checks to see diffs for specific functions.

### 7. Fix non-matching functions

For each non-matching function, you have **up to 3 attempts**: rebuild and run `tools/check --no-pager <func>` after each change. If it still doesn't match after 3 tries, add a comment above the function:

```cpp
// NON_MATCHING: <explanation of what's wrong>
```

### 8. Final cleanup and verification

- Run `tools/check-format.py` and fix every formatting issue it reports.
- Run `tools/check` one final time to confirm all statuses.
- Use sead math/vector inlines wherever possible; write it as a programmer would.
- **Always verify your contribution is correct** before considering a class done: check that statuses in `file_list.yml` are accurate, that no functions were skipped, and that the build is clean.

## Common Non-Matching Patterns

These patterns come up repeatedly. Recognising them saves iteration time.

**`ldp w8, w9` vs `ldr x8` + `lsr x9`** — When copying adjacent `s32` + `f32` fields the original uses `ldp` (two 32-bit registers). Clang may instead do a 64-bit `ldr` + `lsr #0x20`. This is a one-instruction size mismatch (NonMatchingMinor). Reordering source assignments does not fix it — mark NON_MATCHING.

**`stur xzr` zero-init ordering** — In constructors the original often schedules zero-init stores to fill instruction latency slots, producing a different order than C++ declaration order. Rearranging body assignments rarely helps because the compiler re-schedules them. Mark NON_MATCHING.

**Branch layout / shared tail blocks** — The original compiler sometimes places a shared "LABEL" block at the end of a function, reached from two paths via fall-through and one conditional branch. Clang generates the same logic inline with an inverted branch, resulting in a longer function and missing the tail block. This produces a "wrong function size" error. It is NonMatchingMajor if 10+ instructions differ, NonMatchingMinor if only 1–2 instructions differ. Mark NON_MATCHING with a clear explanation.

**`bool` field byte stores** — A `bool` field always generates `strb wzr` (1 byte). The original may instead use `stur xzr` covering the bool plus adjacent memory. You cannot force the wider store from C++; mark NON_MATCHING if it matters.

**`reset()` clearing two fields** — If IDA shows `reset` zeroes a 64-bit slot (e.g. `str xzr, [x0, #0x28]`), the function clears two adjacent 32-bit fields together. The source should assign both to `0`.

**Virtual `reset()` call from record functions** — `recordJudgeAndReset` / `recordCooperateAndReset` call `reset()` through the vtable (`br x1`). This is virtual dispatch, which matches as long as the class is not `final`. Do not devirtualize.

**`*((_DWORD*)this + N)` offset arithmetic** — IDA uses dword (4-byte) indices. `*((_DWORD*)this + N)` = byte offset `N*4`. Qword index `*((_QWORD*)this + N)` = byte offset `N*8`. Always double-check by computing the byte offset; confusing the two is the most common layout mistake.

**Register allocation / wrong function size** — The original compiler sometimes saves more callee-saved registers (e.g. x23/x24/x25) than our clang produces, making the stack frame larger and causing a "wrong function size" error even when the logic is identical. This is NonMatchingMinor. You cannot fix it from C++ source — mark NON_MATCHING and move on. Diagnosis: if the diff shows only register name differences and a frame size difference (e.g. `stp x24, x23` vs absent), it is purely register allocation.

**`STRH` for two adjacent bool fields** — The original compiler sometimes emits a single `strh w8, [x0, #off]` (halfword store of 0x0101) to initialise two adjacent `bool` fields at once. Our compiler emits two `strb` instructions. This is NonMatchingMinor in constructors; mark NON_MATCHING.

**Hardcoded Japanese name strings** — Some classes pass a hardcoded Japanese string literal as the `name` argument to their base class, to `initDemoAnimCamera`, or to `al::NerveExecutor`. These live in rodata and are visible in the constructor disassembly as `ADRL X1, byte_XXXXXXXX`. To find the string, use `py_eval` with `idaapi.get_bytes(addr, N)` and decode as UTF-8. **Always read enough bytes to see the full null terminator** — strings often have unexpected suffixes or brackets (e.g. `"電撃ライン[ライトニング用]"`) and the format checker will reject the source if any character is missing.

**Adjacent f32 store coalescing** — When two adjacent `f32` fields are initialised in sequence (e.g. `mRotateSpeed = 5.0f; mRotateAngle = 0.0f`), the original compiler may merge them into a single 64-bit `str x8` where the upper 32 bits are zero. Our compiler emits two separate word stores. Fix: ensure both fields are written in the source (even if one is already zero) and try reordering them to match the target's store order. The coalescing is sensitive to declaration order and assignment order.

**`sead::Vector3f` copy store ordering** — When the original copies a `sead::Vector3f` with the pattern:
```asm
ldr w9, [x1, #0x8]       ; load source.z
str w9, [x0, #offset+8]  ; store dest.z first
ldr x9, [x1]             ; load source.x+y as 64-bit pair
str x9, [x0, #offset]    ; store dest.x+y together
```
use `mVec.set(other)` instead of `mVec = other`. The `set()` method produces a different store schedule (z stored first, then x+y coalesced as one 64-bit store) that matches the original. Using `mVec = other` emits individual x, y, z stores in field order and will not match.

**Local `sead::Vector3f` stack slot ordering** — When a function has two local `sead::Vector3f` variables and the stack slots are swapped relative to the original, swap their declaration order. Declaring the output vector first (before the input) can swap their stack slot assignments to match the original. For example, if the original places `frontDir` at `sp+0x10` and `shotDir` at `sp+0x20`, declare `shotDir` before `frontDir` in source.

**Load/store scheduling across call boundaries** — The original compiler sometimes hoists a field load or function call ahead of adjacent computation. For example, loading `mElectricLine` and calling `getTrans(this)` immediately after `rotateVectorDegree` (before computing the offset vector) matches the original's schedule. Add local variables to force evaluation order:
```cpp
// Force the pointer load and getTrans before the multiply
BossRaidElectricLine* electricLine = mElectricLine;
const sead::Vector3f& trans = al::getTrans(this);
sead::Vector3f offset = mRadius * shotDir;  // multiply happens after
electricLine->shot(trans, offset);
```

**Extra callee-saved register / wrong function size** — If the diff shows our function saving fewer callee-saved registers than the target (e.g. target saves `x21` but ours does not), the fix is often to remove a redundant local variable that is keeping the register live. For instance, caching a member field into a local `s32 timer = mField` and then using `timer` everywhere can force an extra register; using `mField` directly throughout instead lets the compiler reuse scratch registers without needing an extra save.

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
- Local variables and parameters: `camelCase`.
- Class member variables: `mCamelCase`. Static members: `sCamelCase`. Globals: `gCamelCase`.
- Class member order: `public` → `protected` → `private`; constructor/destructor/operators/other functions, then non-static variables, statics before non-statics.
- Use `override` not `virtual` when overriding. Mark `const` where applicable. No `this->` unless necessary.
- `= default;` for empty constructors/destructors.
- Virtual function order must match the original executable's vtable order.
- Never use `float` or `char16_t` directly — use the sead equivalents `f32` and `char16`. The format checker enforces this. Similarly use `sead::Quatf` not `sead::Quat<float>`, `sead::Vector3f` not `sead::Vector3<float>`.
- **No `snake_case` anywhere** — locals, parameters, everything is `camelCase`. There are no exceptions in this project.
