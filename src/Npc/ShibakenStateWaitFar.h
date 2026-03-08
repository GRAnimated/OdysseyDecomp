#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

class Shibaken;
class ShibakenStateSit;
class ShibakenStateTurn;

class ShibakenStateWaitFar : public al::NerveStateBase {
public:
    ShibakenStateWaitFar(const char* name, Shibaken* shibaken);

    bool tryStart();
    void appear() override;
    void exeTurn();
    void exeSit();

private:
    Shibaken* mShibaken = nullptr;
    ShibakenStateTurn* mStateTurn = nullptr;
    ShibakenStateSit* mStateSit = nullptr;
    sead::Vector3f mWallNormal = sead::Vector3f(0, 0, 0);
};

static_assert(sizeof(ShibakenStateWaitFar) == 0x40);
