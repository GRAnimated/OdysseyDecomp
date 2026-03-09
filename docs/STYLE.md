# OdysseyDecomp — Style Guide

Quick-reference style guide derived from `Contributing.md` and master-branch code.

---

## Language & Formatting

- **C++17**. 4-space indent. Max 100 chars per line.
- Use `clang-format` before committing (`git clang-format`).

---

## Types

| Use | Never |
|---|---|
| `f32` | `float` |
| `s32` | `int` (for sized integers) |
| `u32` | `unsigned int` |
| `char16` | `char16_t` |
| `nullptr` | `NULL` / `0` |
| `sead::Vector3f` | `sead::Vector3<f32>` |
| `sead::Quatf` | `sead::Quat<f32>` |
| `sead::Matrix34f` | `sead::Matrix34<f32>` |

---

## Naming

| Entity | Convention | Example |
|---|---|---|
| Types / Classes / Enums | `UpperCamelCase` | `SphinxRide` |
| Compile-time constants | `UpperCamelCase` | `constexpr s32 MaxCount = 8;` |
| Functions | `camelCase` | `void exeWait()` |
| Local variables | `camelCase` | `s32 stepCount = 0;` |
| Function parameters | `camelCase` | `void init(const ActorInitInfo& info)` |

> **Note**: `Contributing.md` mentions `snake_case` for locals, but the actual
> codebase overwhelmingly uses `camelCase`. Follow the codebase convention.
| Class member variables | `m` + `CamelCase` | `s32 mStepCount;` |
| Struct member variables | `camelCase` (no prefix) | `s32 stepCount;` |
| Static member variables | `s` + `CamelCase` | `static bool sIsInit;` |
| Global variables | `g` + `CamelCase` | `bool gIsDebug;` |

---

## Includes

Three groups separated by blank lines:

```cpp
#include <basis/seadTypes.h>          // 1. System / sead / library (angle brackets)
#include <math/seadVector.h>

#include "Library/LiveActor/ActorInitUtil.h"  // 2. al / Project (quotes)
#include "Library/Nerve/NerveUtil.h"

#include "Npc/SphinxRide.h"           // 3. Game headers (quotes)
#include "Util/SensorMsgFunction.h"
```

---

## Headers

- `#pragma once` — first line.
- Forward-declare types when possible; include only when the full definition is needed.
- No offset comments (`// 0x108`) in headers.
- Functions in **ascending offset order** matching `file_list.yml`.
- `static_assert(sizeof(MyClass) == 0xNN)` when size is known.

---

## Classes

### Access specifier order
`public:` → `protected:` → `private:`

### Member order (within each specifier)
1. Constructors / destructors
2. Operators
3. Other member functions
4. Static member variables
5. Non-static member variables

### Virtual functions
- `override` — when overriding (never rewrite `virtual`).
- `virtual` — only on base declarations.
- Virtual function order must match the executable's vtable.

### Macros
`SEAD_RTTI_BASE`, `SEAD_SINGLETON_DISPOSER`, etc. go right after `{`, before `public:`.

### Constructors / Destructors
- `= default` for empty bodies.
- Prefer in-class default member initializers over initializer lists when the
  value is always the same.

---

## Nerve / State-Machine Pattern

```cpp
namespace {
NERVE_IMPL(MyClass, Idle);
NERVE_IMPL(MyClass, Move);

NERVES_MAKE_STRUCT(MyClass, Idle, Move);       // with struct
NERVES_MAKE_NOSTRUCT(MyClass, Idle, Move);     // without struct
}  // namespace
```

State execution functions: `void exeIdle();`

---

## sead Math — Operator Usage

**Component-wise arithmetic on sead types is always wrong.** Use operators:

```cpp
sead::Vector3f dir = a - b;           // not dir.x = a.x - b.x; ...
result += a - b;                       // not result.x += a.x - b.x; ...
vel.setCross(a, b);                    // not manual cross product
mtx.setTranslation(v);                // not mtx.m[i][3] = v.x; ...
```

---

## Miscellaneous

- `auto` only when type is obvious, too long, or doesn't matter.
- Integer zero-check: `if (value == 0)`, not `if (!value)`. Booleans exempt.
- No `this->` unless necessary.
- `const` on member functions that don't modify state.
- Forward-declare `ActorInitInfo` as `struct` (not `class`).
