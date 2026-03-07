# Common Non-Matching Patterns

Quick reference for patterns that cause assembly mismatches.

---

## Optimizations
| Pattern | Symptom |
|---|---|


## Constructors

**Two adjacent `f32` fields not coalescing into one `str x8`**
Ensure both fields are assigned in source (even if zero) and try reordering them to match the target's store order.

## `sead` Types

### Initializers

sead's math types are initialized in many different ways, which can matter
for matching.

Your decompiler may produce code like:
```cpp
*(this + 0x2F) = 0x3F80000000000000LL;
*(this + 0x2E) = 0;
```
Without context from the assembly, this may be either:
```cpp
sead::Quatf _2e = {0.0f, 0.0f, 0.0f, 1.0f}; // or sead::Quatf(0.0f, 0.0f, 0.0f, 1.0f);
sead::Quatf _2e = sead::Quatf::unit;
```
If the assembly does an `ADRL`/`ADRP` load, it's loading the latter.

### Setting vector types

TODO: Figure out the actual patterns here

**operator= vs .set()**

```cpp
// pseudocode
*(_DWORD *)(v1 + 0x1A0) = *(_DWORD *)(v2 + 0x38);
*(_QWORD *)(v1 + 0x198) = *(_QWORD *)(v2 + 0x30);
// cpp
_198 = v2->_30;
_198.set(v2->_30);
```


**Distance calculations**

Symptom: `ldr s2` (z component) appears after the first two `fsub`s instead of before, and/or callee-saved float regs (`d8`/`d9`/`d10`) are missing from the frame.

```asm
; Target
bl      getPlayerPos
ldp     s8, s9, [x0]      ; load all 3 player pos components into callee-saved regs
ldr     s10, [x0, #0x8]
bl      getTrans
ldp     s0, s1, [x0]      ; load all 3 trans components
ldr     s2, [x0, #0x8]    ; <-- z loaded before any fsub
fsub    s1, s9, s1
fsub    s0, s8, s0
fsub    s2, s10, s2
```

```cpp
// Wrong: temporary changes register pressure
if ((a - b).length() < threshold) ...
// Wrong: reference to getTrans delays load of z component
sead::Vector3f playerPos = getPlayerPos(x);
sead::Vector3f diff = playerPos - getTrans(x);
// Right: copy both into plain locals so all components load before subtracting
sead::Vector3f playerPos = getPlayerPos(x);
sead::Vector3f trans = getTrans(x);
sead::Vector3f diff = playerPos - trans;
if (diff.length() < threshold) ...
```

**Cross product: use `setCross`**
```cpp
// Wrong: individual component writes produce wrong schedule
vel.x = a.y*b.z - a.z*b.y; vel.y = ...; vel.z = ...;
// Right
vel.setCross(a, b);
```

**Vector differences: use `operator+` and `operator-`**
```cpp
dir.x = a.x - b.x;
dir.y = a.y - b.y;
dir.z = a.z - b.z;
// can almost always be replaced with
sead::Vector3f dir = a - b;
```

**Accumulating differences: use `operator+=` and `operator-`**
```cpp
// Wrong: individual ldr/fadd/str
dir.x += a.x - b.x; ...   
dir += getTrans(a) - getTrans(b);  // Right: ldp pattern
```

**Negation: avoid unary `operator-`**
```cpp
sead::Vector3f neg = -vel;                      // may materialise temp differently
sead::Vector3f neg = {-vel.x, -vel.y, -vel.z};  // explicit struct literal
```

**`operator*` on a named variable breaks match; passing as argument is fine**
```cpp
sead::Vector3f scaled = diff * scale;   // wrong if stored to named variable
shot(trans, diff * scale);              // fine when passed directly
```

**`sead::Matrix34f` translation column — use `setTranslation()`**
```cpp
mtx.m[0][3] = v.x; mtx.m[1][3] = v.y; mtx.m[2][3] = v.z;  // wrong
mtx.setTranslation(v);                                    // right — ldp+str pattern
```

### Loops

```cpp
auto* group = new al::DeriveActorGroup<Foo>("name", count);
mGroup = group;
for (s32 i = 0; i < group->getMaxActorCount(); i++) { ... }
// Using mGroup inside the loop reloads from memory; the local keeps the register live
```

---

### Constants & Literals

| IDA shows | Write as |
|---|---|
| `3.4028235e+38` (FLT_MAX) | `sead::Mathf::maxNumber()` |
| `6.2832f` | `sead::Mathf::pi2()` |
| radian value (e.g. `0.40143f`) | `sead::Mathf::deg2rad(23.0f)` |
| `-0.083333f` | `-1.0f / 12.0f` |
| hex like `0xA0`, `0x73` in nerve/interval calls | decimal (`160`, `115`) |

---

### Control Flow & Register Pressure

**Load member pointer after bulk stores, not before**
```cpp
// Wrong: pointer stays live across bulk stores, forces extra callee-saved reg
MyActor* actor = mActor;
mMtx.setTranslation(pos);
actor->doThing();

// Right
mMtx.setTranslation(pos);
mActor->doThing();
```

**Force evaluation order with local variables**
```cpp
// When original hoists a load before a computation:
MyClass* ptr = mMember;           // load first
const sead::Vector3f& t = al::getTrans(this);  // then getTrans
sead::Vector3f offset = mRadius * dir;         // multiply after
ptr->shot(t, offset);
```

**Unsigned comparison to eliminate a branch**
```cpp
// When assembly shows b.lo / b.hs after a modulo:
if ((u32)level < 3) return sTable[level];
return sTable[2];  // fallthrough; don't return a raw literal
```

### Local Vector Stack Slot Order

When two local `sead::Vector3f` variables have swapped stack slots, swap their declaration order. The compiler assigns stack slots in declaration order.

```cpp
// If original has shotDir at sp+0x10, frontDir at sp+0x20:
sead::Vector3f frontDir;  // declared first to lower address
sead::Vector3f shotDir;
```

### Miscellaneous

**IDA dword/qword index arithmetic**:
`*((_DWORD*)this + N)` = byte offset `N×4`
`*((_QWORD*)this + N)` = byte offset `N×8`

**Pre-compute IUseCollision / IUseCamera cast before function calls**

When the target assembly pre-computes the multiple-inheritance pointer adjustment
(`add xN, x19, #0x38`) before a preceding `bl` call, but our compiler delays it
until right before the call that uses it:

```cpp
// Wrong: compiler defers the cast
sead::Vector3f arrowStart = al::getTrans(this) + sArrowCheckOffset;
alCollisionUtil::checkStrikeArrow(this, arrowStart, ...);

// Right: pre-compute the cast so the add appears before getTrans
const al::IUseCollision* collision = this;
sead::Vector3f arrowStart = al::getTrans(this) + sArrowCheckOffset;
alCollisionUtil::checkStrikeArrow(collision, arrowStart, ...);
```

The target compiler generates `cmp x19, #0; add x8, x19, #0x38; csel xN, xzr, x8, eq`
for the null-safe pointer adjustment, and caches the result in a callee-saved register
before calling `getTrans`. Storing the cast result in a local variable forces our
compiler to do the same.

**Pre-load member address before function calls**

When the target hoists `add xN, x19, #offset` before a `bl` call but our
compiler delays it until after:

```cpp
// Wrong: compiler delays the address computation
al::calcJointQuat(&quat, actor, jointName);
sead::Vector3f frontDir = mByeByeLocalAxisFront;  // add comes after bl

// Right: bind the reference early
const sead::Vector3f& frontRef = mByeByeLocalAxisFront;
al::calcJointQuat(&quat, actor, jointName);
sead::Vector3f frontDir = frontRef;
```

Look for diff signature: a `deleted` add instruction before a `bl`, and an
`added` add instruction after the `bl`, targeting the same member offset.

**Inline ternary string argument instead of named local**

Named `const char*` locals assigned from ternaries cause extra register allocation:

```cpp
// Wrong: named variable keeps string pointer in a register too long
const char* action = mIsHack ? "RunHack" : "RunEnemy";
al::startAction(mActor, action);

// Right: inline the ternary
al::startAction(mActor, mIsHack ? "RunHack" : "RunEnemy");
```

**`makeActorAlive()` / `makeActorDead()` vs `appear()` / `kill()`**

These call different vtable slots. If the target uses vtable offset 0x20/0x30,
use `makeActorAlive()`/`makeActorDead()` directly instead of `appear()`/`kill()`.

| Method | Vtable offset |
|---|---|
| `appear()` | 0x18 |
| `makeActorAlive()` | 0x20 |
| `kill()` | 0x28 |
| `makeActorDead()` | 0x30 |

## Return type fixes — void → bool

Some al/rs utility functions are declared `void` in our headers but actually return
`bool` in the binary. When the caller tests the return value (e.g. `tbnz w0, #0`
after the `bl`), the mismatch causes a tail-call optimization difference — the
compiler turns a `bl` + `ret` into a single `b` (tail call) when the return value
is unused.

**How to detect:**  
Disassemble the **caller** in the target binary. If you see `tbnz w0` / `cbz w0`
right after the `bl` to a function we declare as `void`, that function actually
returns `bool`.

**Example:** `rs::sendMsgEventFlowScareCheck` was `void` but actually returns
`bool`. Fixing the return type and adding `if (rs::sendMsgEventFlowScareCheck(…)) return;`
in `TalkNpc::attackSensor` promoted it to Matching.

**Checklist when fixing:**
1. Change the return type in every header that declares the function.
2. Change the implementation (`.cpp`) — add `return` in front of the inner call.
3. Update every call site that should test the return value.
4. Rebuild and `check` — verify no regressions in any caller.

## sead::Vector3f dot product

Use `a.dot(b)` instead of `a.x * b.x + a.y * b.y + a.z * b.z`. The sead inline
generates the same code but keeps the source cleaner for future matching work.
