#include "MapObj/BendLifeTree.h"

#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorMsgFunction.h"
#include "Library/LiveActor/LiveActorUtil.h"
#include "Library/LiveActor/SubActorKeeper.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Yaml/ByamlUtil.h"

#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(BendLeafTree, Wait);

NERVES_MAKE_NOSTRUCT(BendLeafTree, Wait)
}  // namespace

BendLeafTree::BendLeafTree(const char* name) : al::LiveActor(name) {}

void BendLeafTree::init(const al::ActorInitInfo& info) {
    al::initMapPartsActor(this, info, nullptr);
    initBendInfo();
    al::initNerve(this, &Wait, 0);

    sead::Vector3f scale;
    al::getScale(&scale, *al::getPlacementInfo(info));

    mScaleY = scale.y;

    s32 subActorNum = al::getSubActorNum(this);
    if (subActorNum >= 1)
        for (s32 i = 0; i < subActorNum; i++)
            al::setScale(al::getSubActor(this, i), scale);

    s32 mtpAnimFrame = 0;
    if (al::tryGetArg(&mtpAnimFrame, info, "MtpAnimFrame") &&
        al::isMtpAnimExist(this, "ClashWorldHomeTreeMtpAnim")) {
        al::startMtpAnimAndSetFrameAndStop(this, "ClashWorldHomeTreeMtpAnim", mtpAnimFrame);

        if (mNumJoints >= 1) {
            for (s32 i = 0; i < mNumJoints; i++) {
                bool isLeafAnimExist =
                    al::isMtpAnimExist(al::getSubActor(this, i), "ClashWorldHomeLeafMtpAnim");

                al::LiveActor* subActor = al::getSubActor(this, i);

                if (!isLeafAnimExist) {
                    if (al::isMtpAnimExist(subActor, "ClashWorldHomeLeafDownMtpAnim")) {
                        al::startMtpAnimAndSetFrameAndStop(al::getSubActor(this, i),
                                                           "ClashWorldHomeLeafDownMtpAnim",
                                                           mtpAnimFrame);
                    }
                } else
                    al::startMtpAnimAndSetFrameAndStop(subActor, "ClashWorldHomeLeafMtpAnim",
                                                       mtpAnimFrame);
            }
        }
    }

    if (al::isEqualString(getName(), "墜落ワールドホーム木000")) {
        mBendTimers = new s32[3];
        mBendTimers[0] = 0;
        mBendTimers[1] = 0;
        mBendTimers[2] = 0;
    }

    al::offSyncAlphaMaskSubActorAll(this);
    al::calcJointPos(&mLeafAPos, this, "LeafA");
    makeActorAlive();
}

bool BendLeafTree::initBendInfo() {
    if (!al::isExistModelResourceYaml(this, "BendInfo", nullptr))
        return false;

    al::ByamlIter bendIter(al::getModelResourceYaml(this, "BendInfo", nullptr));

    al::ByamlIter iter;
    if (bendIter.tryGetIterByKey(&iter, "Joints")) {
        mNumJoints = iter.getSize();

        if (mNumJoints < 1)
            return false;

        al::initJointControllerKeeper(this, mNumJoints);
        mBendSpeeds = new f32[mNumJoints];
        mBendAngles = new f32[mNumJoints];
        mDitherTimers = new s32[mNumJoints];
        mBendImpulses = new f32[mNumJoints];

        if (mNumJoints >= 1) {
            for (s32 i = 0; i < mNumJoints; i++) {
                const char* str = "";
                iter.tryGetStringByIndex(&str, i);
                al::initJointLocalZRotator(this, &mBendSpeeds[i], str);
                mBendSpeeds[i] = 0.0f;
                mBendAngles[i] = 0.0f;
                mDitherTimers[i] = -1;
                mBendImpulses[i] = 0.0f;
            }
        }

        if (al::tryGetByamlF32(&mBendLength, bendIter, "BendLeafLength")) {
            mBendLength *= mScaleY;
            return true;
        }
    }

    return false;
}

void BendLeafTree::initAfterPlacement() {
    al::tryExpandClippingByDepthShadowLength(this, &mClippingExpansion);
}

void BendLeafTree::exeWait() {
    if (mNumJoints < 1)
        return;

    for (s32 i = 0; i < mNumJoints; i++) {
        f32 v3 = mBendImpulses[i];
        if (v3 < 0.0f) {
            mBendImpulses[i] = -v3;
        } else {
            if (v3 > 0.0f) {
                mBendAngles[i] += (v3 * -3.2f);
                mBendImpulses[i] = 0.0f;
            }
        }
    }
}

// NON_MATCHING: b.le vs b.ls
void BendLeafTree::movement() {
    if ((al::getCameraPos(this, 0) - mLeafAPos).length() <= mScaleY * 3500.0f)
        al::LiveActor::movement();
}

void BendLeafTree::control() {
    for (s32 i = 0; i < mNumJoints; i++) {
        if (mBendImpulses[i] == 0.0f) {
            mBendAngles[i] += (mBendSpeeds[i] * -0.01f);
            if (al::getRandom() < 0.05f)
                mBendAngles[i] += -0.05f;
            mBendSpeeds[i] = mBendSpeeds[i] * 0.92f;
            mBendSpeeds[i] += mBendAngles[i];

            if (mDitherTimers[i] == 0)
                al::restartDitherAnimAutoCtrl(al::getSubActor(this, i));

            if (mDitherTimers[i] >= 0)
                mDitherTimers[i]--;
        }
    }

    if (al::isEqualString(getName(), "墜落ワールドホーム木000")) {
        for (int i = 0; i < 3; i++)
            if (mBendTimers[i] > 0)
                mBendTimers[i]--;
    }
}

// NON_MATCHING: regswap
bool BendLeafTree::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                              al::HitSensor* self) {
    if ((al::getCameraPos(this, 0) - mLeafAPos).length() > (mScaleY * 3500.0f))
        return false;

    if (rs::isMsgPlayerDisregardHomingAttack(message) ||
        rs::isMsgPlayerDisregardTargetMarker(message) || al::isMsgPlayerDisregard(message))
        return true;

    if (al::isEqualString(getName(), "墜落ワールドホーム木000") &&
        (al::isSensorPlayer(other) ||
         (al::isSensorHostName(other, "とりつき帽子") && al::isSensorName(other, "Attack")))) {
        if (al::isSensorName(self, "BodyD")) {
            if (mBendTimers[0] == 0) {
                mBendAngles[3] += -1.2f;
                mBendTimers[0] += 40;
            }
            return false;
        }
        if (al::isSensorName(self, "BodyE")) {
            if (mBendTimers[1] == 0) {
                mBendAngles[4] += -1.2f;
                mBendTimers[1] += 40;
            }
            return false;
        }
        if (al::isSensorName(self, "BodyF")) {
            if (mBendTimers[2] == 0) {
                mBendAngles[5] += -1.2f;
                mBendTimers[2] += 40;
            }
            return false;
        }
    }

    if (rs::isMsgPlayerAndCapObjHipDropAll(message) || rs::isMsgPlayerAndCapHipDropAll(message)) {
        al::LiveActor* sensorHost = al::getSensorHost(self);
        if (mNumJoints >= 1) {
            for (s32 i = 0; i < mNumJoints; i++) {
                if (al::getSubActor(this, i) == sensorHost) {
                    f32 length = (al::getSensorPos(self) - al::getSensorPos(other)).length();
                    mBendAngles[i] += (length * -3.2f) / mBendLength;  // regswap
                    return false;
                }
            }
        }
    }

    if (al::isMsgFloorTouch(message)) {
        al::LiveActor* sensorHost = al::getSensorHost(self);
        if (mNumJoints >= 1) {
            for (s32 i = 0; i < mNumJoints; i++) {
                if (al::getSubActor(this, i) == sensorHost) {
                    f32 length = (al::getSensorPos(self) - al::getSensorPos(other)).length();
                    mBendAngles[i] += ((length + (mScaleY * 100.0f)) * -0.12f) /
                                      (mBendLength + (mScaleY * 100.0f));

                    if (al::isEqualString(getName(), "墜落ワールドホーム木000")) {
                        mDitherTimers[i] = 40;
                        al::stopDitherAnimAutoCtrl(al::getSubActor(this, i));
                        al::setModelAlphaMask(al::getSubActor(this, i), 1.0f);
                    }

                    return true;
                }
            }
        }
    }

    if (rs::isMsgPlayerJumpTakeOffFloor(message)) {
        al::LiveActor* sensorHost = al::getSensorHost(self);
        if (mNumJoints >= 1) {
            for (s32 i = 0; i < mNumJoints; i++) {
                if (al::getSubActor(this, i) == sensorHost) {
                    f32 length = (al::getSensorPos(self) - al::getSensorPos(other)).length();
                    mBendImpulses[i] = -length / mBendLength;
                    break;
                }
            }
        }
    }

    return false;
}

void BendLeafTree::attackSensor(al::HitSensor* self, al::HitSensor* other) {}
