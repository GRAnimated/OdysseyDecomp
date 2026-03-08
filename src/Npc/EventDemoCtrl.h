#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class EventDemoCtrl {
public:
    EventDemoCtrl();
    bool isActiveDemo() const;
    bool isRequestEndDemo() const;
    bool isActiveDemoWithPlayer() const;
    void endDemo();
    const al::LiveActor* getDemoStartActor() const;

private:
    void* _00 = nullptr;
    void* _08 = nullptr;
};

static_assert(sizeof(EventDemoCtrl) == 0x10);
