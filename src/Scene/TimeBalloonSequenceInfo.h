#pragma once

namespace al {
class LiveActor;
}

class TimeBalloonSequenceInfo {
public:
    void setAccessor(al::LiveActor*);
    void addHioNode();
    void disableLayout();
};
