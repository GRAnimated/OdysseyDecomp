#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class LiveActor;
}

class ShibakenMoveAnimCtrl : public al::NerveExecutor {
public:
    ShibakenMoveAnimCtrl(al::LiveActor* actor, const f32& runSpeed, const f32& walkSpeed,
                         const f32& sniffSpeed);

    void update();
    void startWalkSniff();
    void endWalkSniff();

    void exeMove();
    void exeWalkSniffStart();
    void exeWalkSniff();
    void exeWalkSniffEnd();

    al::LiveActor* mActor;
    f32 mWalkRunRatio;
    f32 mWalkSniffRatio;
    f32 mWalkSniffWalkRatio;
    bool mIsSniffing;
    const f32* mRunSpeed;
    const f32* mWalkSpeed;
    const f32* mMaxSpeed;
};

static_assert(sizeof(ShibakenMoveAnimCtrl) == 0x40);
