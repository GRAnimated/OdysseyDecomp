#pragma once

#include <basis/seadTypes.h>

class HackCapTrigger {
public:
    enum EAfterMovementTrigger {
        Trigger0 = 0,
        Trigger1 = 1,
        Trigger2 = 2,
        Trigger3 = 3,
    };

    HackCapTrigger();

    void clearAfterMovemetTrigger();
    void set(EAfterMovementTrigger);
    bool isOn(EAfterMovementTrigger) const;

private:
    u32 mAfterMovementTriggers;
};

static_assert(sizeof(HackCapTrigger) == 4);
