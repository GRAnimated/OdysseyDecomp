#pragma once

#include <basis/seadTypes.h>

namespace al {
class LayoutInitInfo;
}

class MiniGameMenu {
public:
    MiniGameMenu(const al::LayoutInitInfo&);

private:
    u8 _padding[0x38];
};

static_assert(sizeof(MiniGameMenu) == 0x38);
