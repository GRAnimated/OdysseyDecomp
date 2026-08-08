#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

class IUsePlayerHack;

class HackerStateBase : public al::NerveStateBase {
public:
    HackerStateBase(const char* name, al::LiveActor* actor, IUsePlayerHack** playerHack);
    ~HackerStateBase() override = default;

protected:
    al::LiveActor* mActor;
    IUsePlayerHack** mPlayerHack;
};

