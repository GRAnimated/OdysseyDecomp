#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

class PlayerStateCeilingKeep : public al::ActorStateBase {
public:
    PlayerStateCeilingKeep(al::LiveActor* player);
    ~PlayerStateCeilingKeep() override;

    void appear() override;
    void exeKeep();
    void exeLeave();

private:
    sead::Vector3f mCeilingNormal = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mFront = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(PlayerStateCeilingKeep) == 0x38);
