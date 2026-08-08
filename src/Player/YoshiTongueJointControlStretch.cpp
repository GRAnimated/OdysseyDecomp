#include "Player/YoshiTongueJointControlStretch.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MatrixUtil.h"

struct YoshiTongueJointControlStretch::JointInfo {
    s32 jointIndex;
    s32 _4;
    sead::Matrix34f mMtx;
    u8 _38[0xC];
};

void YoshiTongueJointControlStretch::update(const sead::Vector3f& rootPos,
                                             const sead::Vector3f& direction,
                                             const sead::Vector3f& tipPos) {
    al::makeMtxSideUpPos(&mMtx, rootPos, direction, tipPos);
}

// NON_MATCHING: target merges only the scaled box min/max (348 bytes), while the clean public
// BoundBox3 addPoint form performs extra comparisons (440 bytes); next hypothesis is an original
// SEAD box-merge helper absent from the recovered BoundBox3 interface.
void YoshiTongueJointControlStretch::calcBoundingBox(sead::BoundBox3f* boundingBox) const {
    boundingBox->setUndef();

    const sead::Vector3f& actorTrans = al::getTrans(mActor);
    for (const JointInfo& jointInfo : mJointInfos) {
        const sead::Vector3f pos = jointInfo.mMtx.getTranslation() - actorTrans;
        const sead::Vector3f offset(1.0f, 1.0f, 1.0f);
        sead::BoundBox3f jointBox(pos - offset, pos + offset);
        jointBox.scaleX(35.0f);
        jointBox.scaleY(35.0f);
        jointBox.scaleZ(35.0f);
        boundingBox->addPoint(jointBox.getMin());
        boundingBox->addPoint(jointBox.getMax());
    }
}

const char* YoshiTongueJointControlStretch::getCtrlTypeName() const {
    return "伸縮";
}
