#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class AreaObj;
class LiveActor;
}

class WorldEndBorderKeeper : public al::NerveExecutor {
public:
    WorldEndBorderKeeper(const al::LiveActor* actor);
    void reset();
    void update(const sead::Vector3f& position, const sead::Vector3f& velocity, bool isDemo);
    void exeOutside();
    void exeInside();
    void exePullBack();
    void exeWaitBorder();
    const sead::Vector3f& getVelocity() const { return mVelocity; }

    ~WorldEndBorderKeeper() override;

private:
    const al::LiveActor* mActor;
    sead::Vector3f mPosition = sead::Vector3f::zero;
    sead::Vector3f mInputVelocity = sead::Vector3f::zero;
    bool mIsDemo = false;
    al::AreaObj* mAreaObj = nullptr;
    sead::Vector3f mNearestEdgePos = {0.0f, 0.0f, 0.0f};
    f32 mBorderTimer = 0.0f;
    f32 mBorderDistance = 0.0f;
    sead::Vector3f mVelocity = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(WorldEndBorderKeeper) == 0x60);
