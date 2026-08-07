#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

class IUsePlayerHack;

class HackerStateBase : public al::NerveStateBase {
public:
    HackerStateBase(const char*, al::LiveActor*, IUsePlayerHack**);
    ~HackerStateBase() override = default;

protected:
    al::LiveActor* mActor;
    IUsePlayerHack** mPlayerHack;
};

static_assert(sizeof(HackerStateBase) == 0x28);
