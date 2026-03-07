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

    al::LiveActor* mActor = nullptr;
    f32 _18 = 0;
    f32 _1c = 0;
    f32 _20 = 0;
    bool _24 = false;
};
