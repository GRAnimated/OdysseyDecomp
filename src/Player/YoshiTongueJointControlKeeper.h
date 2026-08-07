#pragma once

#include <math/seadBoundBox.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}

class YoshiTongueJointControlStretch;

class YoshiTongueJointControlKeeper {
public:
    YoshiTongueJointControlKeeper(const al::LiveActor* actor, const al::LiveActor* modelActor);

    void update(const sead::Vector3f& rootPos, const sead::Vector3f& direction,
                const sead::Vector3f& tipPos);
    void calcTongueBoundingBox(sead::BoundBox3f* boundingBox) const;

private:
    const al::LiveActor* mActor;
    const al::LiveActor* mModelActor;
    YoshiTongueJointControlStretch* mStretch = nullptr;
};

static_assert(sizeof(YoshiTongueJointControlKeeper) == 0x18);
