#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class TimeBalloonSequenceInfo {
public:
    void setAccessor(al::LiveActor*);
    void addHioNode();
    void disableLayout() { mIsLayoutEnabled = false; }

private:
    unsigned char _0[0x30];
    bool mIsLayoutEnabled = true;
};
