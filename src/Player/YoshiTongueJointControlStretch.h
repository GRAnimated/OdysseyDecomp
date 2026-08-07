#pragma once

#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {
class LiveActor;
}

class YoshiTongueJointControlStretch : public al::JointControllerBase {
public:
    YoshiTongueJointControlStretch(const al::LiveActor* actor);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* jointMtx) override;
    const char* getCtrlTypeName() const override;

    void update(const sead::Vector3f& rootPos, const sead::Vector3f& direction,
                const sead::Vector3f& tipPos);
    void calcBoundingBox(sead::BoundBox3f* boundingBox) const;

private:
    u8 _28[0x48]{};
};

static_assert(sizeof(YoshiTongueJointControlStretch) == 0x70);
