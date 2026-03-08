#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseSceneObjHolder;
}

class InputSeparator {
public:
    InputSeparator(const al::IUseSceneObjHolder*, bool);

private:
    u8 _padding[0x18];
};

static_assert(sizeof(InputSeparator) == 0x18);
