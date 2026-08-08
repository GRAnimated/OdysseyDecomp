#pragma once

#include <container/seadPtrArray.h>
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
    void update(const sead::Vector3f& rootPos, const sead::Vector3f& direction,
                const sead::Vector3f& tipPos);

    void calcBoundingBox(sead::BoundBox3f* boundingBox) const;
    const char* getCtrlTypeName() const override;

private:
    struct JointInfo;

    const al::LiveActor* mActor;
    sead::PtrArray<JointInfo> mJointInfos;
    sead::Matrix34f mMtx;
};

static_assert(sizeof(YoshiTongueJointControlStretch) == 0x70);
