#pragma once

#include <basis/seadTypes.h>

namespace sead {
template <typename T>
struct Vector3;
using Vector3f = Vector3<f32>;
}  // namespace sead

class NpcStateWaitParam {
public:
    NpcStateWaitParam(const char* action, const char*, const char*, const char*, const char*,
                      const char*, bool, const sead::Vector3f*, bool);

    const char* mAction;

private:
    u8 _8[0x40];
};

static_assert(sizeof(NpcStateWaitParam) == 0x48);
