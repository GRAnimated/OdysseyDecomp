#pragma once

#include <basis/seadTypes.h>

namespace al {
class LayoutActor;
}

class InputSeparator {
public:
    InputSeparator(al::LayoutActor*, s32);

private:
    u8 _padding[0x18];
};

static_assert(sizeof(InputSeparator) == 0x18);
