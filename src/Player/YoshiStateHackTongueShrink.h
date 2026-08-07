#pragma once

#include <basis/seadTypes.h>

class YoshiStateHackTongueShrink {
public:
    bool isEnableAccelForceRun() const;
    u32 getLoopRunCount() const;

private:
    unsigned char _0[0x100];
    u32 mLoopRunCount;
};
