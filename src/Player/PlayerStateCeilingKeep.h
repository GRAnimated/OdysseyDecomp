#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

class PlayerStateCeilingKeep : public al::ActorStateBase {
public:
    PlayerStateCeilingKeep(al::LiveActor* player);

    void appear() override;
    void exeKeep();
    void exeLeave();
    ~PlayerStateCeilingKeep() override;

private:
    sead::Vector3f mCeilingNormal = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mFront = {0.0f, 0.0f, 0.0f};
};

