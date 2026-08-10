#include "Player/YoshiTongueJointControlStretch.h"

#include <prim/seadSafeString.h>

#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"

namespace {
constexpr f32 cJointRates[] = {0.0f, 0.15f, 0.3f, 0.5f, 0.7f, 0.8f, 0.9f, 1.0f};
}

struct YoshiTongueJointControlStretch::JointInfo {
    JointInfo(s32 jointIndex, f32 rate) : jointIndex(jointIndex), rate(rate) {}

    s32 jointIndex;
    f32 rate;
    sead::Matrix34f mMtx = sead::Matrix34f::ident;
    sead::Vector3f position{0.0f, 0.0f, 0.0f};
};

YoshiTongueJointControlStretch::YoshiTongueJointControlStretch(const al::LiveActor* actor)
    : al::JointControllerBase(16), mActor(actor) {
    const al::ModelKeeper* modelKeeper = actor->getModelKeeper();
    mJointInfos.allocBuffer(8, nullptr, 8);

    for (s32 i = 0; i < 8; i++) {
        sead::FixedSafeString<0x40> jointName;
        jointName.format("%s%02d", "Tongue", i + 1);

        JointInfo* jointInfo =
            new JointInfo(al::getJointIndex(modelKeeper, jointName.cstr()), cJointRates[i]);
        appendJointId(jointInfo->jointIndex);
        mJointInfos.pushBack(jointInfo);
    }
}

// NON_MATCHING: 1108 bytes vs target 1112 with exact 9/9 semantic direct calls; inlining the
// one-use Hermite tangents recovers most target register pressure while the validated vector form
// still uses a 0xC0 frame instead of target 0xE0. Next hypothesis: original root/vector lifetime
// that creates the final stack/callee-save slot.
void YoshiTongueJointControlStretch::calcJointCallback(s32 jointIndex, sead::Matrix34f* jointMtx) {
    JointInfo* rootJoint = mJointInfos.front();
    if (rootJoint->jointIndex == jointIndex) {
        const sead::Vector3f jointTrans = jointMtx->getTranslation();
        const sead::Vector3f tongueTrans = mMtx.getTranslation();
        const sead::Vector3f actorTrans = al::getTrans(mActor);
        const f32 jointDistance = (jointTrans - actorTrans).length();
        const f32 tongueDistance = (tongueTrans - actorTrans).length();
        rootJoint->mMtx = *jointMtx;
        const sead::Vector3f rootPos =
            actorTrans + jointMtx->getBase(0) * sead::Mathf::min(jointDistance, tongueDistance);
        rootJoint->mMtx.setTranslation(rootPos);

        if (tongueDistance >= jointDistance) {
            for (s32 i = 0; i < mJointInfos.size(); i++) {
                JointInfo* info = mJointInfos[i];
                al::hermiteVec(&info->position, rootPos, rootJoint->mMtx.getBase(0) * 300.0f,
                               tongueTrans, mMtx.getBase(0) * 100.0f, info->rate);
            }
        } else {
            for (s32 i = 0; i < mJointInfos.size(); i++) {
                JointInfo* info = mJointInfos[i];
                al::lerpVec(&info->position, rootPos, tongueTrans, info->rate);
            }
        }

        sead::Vector3f direction = mJointInfos[1]->position - rootPos;
        if (!al::tryNormalizeOrZero(&direction))
            direction = rootJoint->mMtx.getBase(0);
        al::turnMtxXDirDegree(&rootJoint->mMtx, rootJoint->mMtx, direction, 360.0f);
        rootJoint->mMtx.setTranslation(rootPos);

        for (s32 i = 0; i < mJointInfos.size() - 2; i++) {
            JointInfo* current = mJointInfos[i];
            JointInfo* next = mJointInfos[i + 1];
            direction = mJointInfos[i + 2]->position - next->position;
            if (!al::tryNormalizeOrZero(&direction))
                direction = current->mMtx.getBase(0);
            al::turnMtxXDirDegree(&next->mMtx, current->mMtx, direction, 360.0f);
            next->mMtx.setTranslation(next->position);
        }

        mJointInfos.back()->mMtx = mMtx;
    }

    for (s32 i = 1; i < mJointInfos.size(); i++) {
        const JointInfo* info = mJointInfos[i];
        if (info->jointIndex == jointIndex) {
            *jointMtx = info->mMtx;
            return;
        }
    }
}

void YoshiTongueJointControlStretch::update(const sead::Vector3f& rootPos,
                                             const sead::Vector3f& direction,
                                             const sead::Vector3f& tipPos) {
    al::makeMtxSideUpPos(&mMtx, rootPos, direction, tipPos);
}

void YoshiTongueJointControlStretch::calcBoundingBox(sead::BoundBox3f* boundingBox) const {
    boundingBox->setUndef();

    const sead::Vector3f actorTrans = al::getTrans(mActor);
    sead::Vector3f* boxMin = const_cast<sead::Vector3f*>(&boundingBox->getMin());
    sead::Vector3f* boxMax = const_cast<sead::Vector3f*>(&boundingBox->getMax());
    const s32 count = mJointInfos.size();
    if (count < 1)
        return;

    f32 minX = boxMin->x;
    u64 i = 0;
    do {
        const JointInfo* jointInfo = mJointInfos.data()[i];
        const sead::Vector3f pos = jointInfo->mMtx.getTranslation() - actorTrans;
        const sead::Vector3f offset(1.0f, 1.0f, 1.0f);
        sead::BoundBox3f jointBox(pos - offset, pos + offset);
        jointBox.scaleX(35.0f);
        jointBox.scaleY(35.0f);
        jointBox.scaleZ(35.0f);

        const sead::Vector3f& jointMin = jointBox.getMin();
        const sead::Vector3f& jointMax = jointBox.getMax();
        if (minX > jointMin.x) {
            minX = jointMin.x;
            boxMin->x = jointMin.x;
        }
        if (boxMin->y > jointMin.y)
            boxMin->y = jointMin.y;
        if (boxMin->z > jointMin.z)
            boxMin->z = jointMin.z;
        if (boxMax->x < jointMax.x)
            boxMax->x = jointMax.x;
        if (boxMax->y < jointMax.y)
            boxMax->y = jointMax.y;
        if (boxMax->z < jointMax.z)
            boxMax->z = jointMax.z;
        i++;
    } while (count != static_cast<s32>(i));
}

const char* YoshiTongueJointControlStretch::getCtrlTypeName() const {
    return "伸縮";
}
