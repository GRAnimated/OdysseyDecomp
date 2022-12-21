#pragma once

#include <framework/seadGameFrameworkNx.h>

namespace al {
    class GameFrameworkNx : public sead::GameFrameworkNx {
    public:
        char size[0x278 - 0xA0];
    };
}    // namespace al