#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class LiveActor;
}

class WorldEndBorderKeeper : public al::NerveExecutor {
public:
    WorldEndBorderKeeper(const al::LiveActor* actor);
    ~WorldEndBorderKeeper() override;

    void reset();
    void update(const sead::Vector3f& position, const sead::Vector3f& velocity, bool isDemo);
    void exeOutside();
    void exeInside();
    void exePullBack();
    void exeWaitBorder();
    const sead::Vector3f& getVelocity() const { return mVelocity; }

private:
    u8 mPadding[0x54 - sizeof(al::NerveExecutor)];
    sead::Vector3f mVelocity;
};

static_assert(sizeof(WorldEndBorderKeeper) == 0x60);
