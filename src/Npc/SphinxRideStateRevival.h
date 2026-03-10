#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
}

class SphinxRide;

class SphinxRideStateRevival : public al::HostStateBase<SphinxRide> {
public:
    SphinxRideStateRevival(SphinxRide* host, const al::ActorInitInfo& initInfo, bool isFromTrans);

    void appear() override;
    void exeRevival();
    bool isNoMove();

private:
    sead::Vector3f mRevivalTrans = sead::Vector3f::zero;
    sead::Vector3f mRevivalRotate = sead::Vector3f::zero;
};

static_assert(sizeof(SphinxRideStateRevival) == 0x38);
