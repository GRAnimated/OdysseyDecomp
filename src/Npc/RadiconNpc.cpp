#include "Npc/RadiconNpc.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraUtil.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointLookAtController.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "MapObj/RouteGuideDirector.h"
#include "Npc/NpcStateReaction.h"
#include "Npc/NpcStateReactionParam.h"
#include "Npc/RadiconCar.h"
#include "Npc/RandomActionUpdater.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/TalkNpcParam.h"
#include "Player/CapTargetInfo.h"
#include "Player/PlayerHackStartShaderCtrl.h"
#include "Util/DemoUtil.h"
#include "Util/Hack.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
using namespace al;

NERVE_IMPL(RadiconNpc, Wait)
NERVE_IMPL(RadiconNpc, Reaction)
NERVE_HOST_TYPE_IMPL(RadiconNpc, WaitHack)
NERVE_HOST_TYPE_IMPL(RadiconNpc, StartHack)
NERVE_HOST_TYPE_IMPL(RadiconNpc, EndHack)
NERVE_HOST_TYPE_IMPL(RadiconNpc, HackWait)
NERVE_HOST_TYPE_IMPL(RadiconNpc, HackShake)

struct {
    RadiconNpcNrvWait Wait;
    RadiconNpcNrvReaction Reaction;
    HostTypeNrvWaitHack WaitHack;
    HostTypeNrvStartHack StartHack;
    HostTypeNrvEndHack EndHack;
    HostTypeNrvHackWait HackWait;
    HostTypeNrvHackShake HackShake;
    PlayerHackStartShaderParam sPlayerHackStartShaderParam{false, 250.0f, 10, 20};
} NrvRadiconNpc;
}  // namespace

RadiconNpc::RadiconNpc(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: regswap (x24/x25 vs x24/x23), stack layout differences
void RadiconNpc::init(const al::ActorInitInfo& initInfo) {
    mActionAnimInfo = new TalkNpcActionAnimInfo();
    mActionAnimInfo->mWaitActionName = "RadiConWait";
    al::initActorWithArchiveName(this, initInfo, "RadiCon", "CityMan");
    al::calcQuat(&mInitQuat, this);
    mParam = rs::initTalkNpcParam(this, nullptr);
    al::startMclAnim(this, "SkinWhiteEyeBlue");
    mActionAnimInfo->init(this, initInfo, mParam, nullptr);

    mRandomActionUpdater = new RandomActionUpdater(this, mActionAnimInfo);
    mRandomActionUpdater->initBalloonAction("RadiConShakeBalloon");

    al::initJointControllerKeeper(this, 1);
    mJointLookAtController = al::initJointLookAtController(this, 1);
    reinterpret_cast<u8*>(mJointLookAtController)[0x52] = 1;

    sead::Vector2f angleLimits = {-40.0f, 40.0f};
    sead::Vector2f angleLimits2 = {-0.0f, 0.0f};
    al::appendJointLookAtController(mJointLookAtController, this, "Spine1", 0.1f, angleLimits,
                                    angleLimits2, sead::Vector3f::ez, sead::Vector3f::ex);

    al::initNerve(this, &NrvRadiconNpc.Wait, 8);

    mNpcStateReaction = NpcStateReaction::createForHackActor(this, nullptr);
    auto* reactionParam = new NpcStateReactionParam("RadiConReaction", "RadiConReactionCap");
    mNpcStateReaction->mParam = reactionParam;
    mNpcStateReaction->_29 = true;
    al::initNerveState(this, mNpcStateReaction, &NrvRadiconNpc.Reaction, "リアクション");

    mCapTargetInfo = rs::createCapTargetInfo(this, "RadiCon");
    mCameraTicket =
        al::initObjectCamera(this, initInfo, "ラジコンカメラ", "固定");

    auto* car = new RadiconCar(&mPlayerHack);
    mRadiconCar = car;
    if (al::isExistLinkChild(initInfo, "LinkTargetRadiConCar", 0)) {
        al::initLinksActor(car, initInfo, "LinkTargetRadiConCar", 0);
    } else {
        car->init(initInfo);
        const sead::Vector3f& trans = al::getTrans(this);
        const sead::Vector3f& front = al::getFront(this);
        sead::Vector3f carPos = {front.x * 400.0f + trans.x, front.y * 400.0f + trans.y,
                                 front.z * 400.0f + trans.z};
        al::resetPosition(car, carPos);
    }

    mEventFlowExecutor =
        rs::initEventFlowSuffix(this, initInfo, "RadiCon", "RadiConNpc", "RadiConNpc");
    rs::initEventCharacterName(mEventFlowExecutor, initInfo, "RadiConNpc");

    mPlayerHackStartShaderCtrl =
        new PlayerHackStartShaderCtrl(this, &NrvRadiconNpc.sPlayerHackStartShaderParam);

    makeActorAlive();
}

void RadiconNpc::initAfterPlacement() {
    rs::tryRegisterRadiConNpcToRadiConRaceWatcher(this);
}

void RadiconNpc::kill() {
    if (al::isActiveCamera(mCameraTicket))
        al::endCamera(this, mCameraTicket, -1, false);
    al::LiveActor::kill();
}

void RadiconNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    rs::attackSensorNpcCommon(self, other);
}

// NON_MATCHING: regswap (x24 vs x21 for &mPlayerHack)
bool RadiconNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                            al::HitSensor* self) {
    if (al::isMsgPlayerDisregard(msg))
        return !al::isSensorName(self, "Head");

    if (rs::isMsgPlayerDisregardTargetMarker(msg))
        return true;

    if (al::isNerve(this, &NrvRadiconNpc.Wait) ||
        (al::isNerve(this, &NrvRadiconNpc.Reaction) && !al::isNewNerve(this))) {
        bool isInvalid = rs::isInvalidTrampleSensor(self, mParam);
        bool handled;
        if (isInvalid)
            handled = mNpcStateReaction->receiveMsgWithoutTrample(msg, other, self);
        else
            handled = mNpcStateReaction->receiveMsg(msg, other, self);
        if (handled) {
            al::setNerve(this, &NrvRadiconNpc.Reaction);
            return true;
        }
    }

    if (mNpcStateReaction->receiveMsgNoReaction(msg, other, self))
        return true;

    if (rs::isMsgCapStartLockOn(msg) && !al::isNerve(this, &NrvRadiconNpc.WaitHack) &&
        !al::isNerve(this, &NrvRadiconNpc.HackWait) &&
        !al::isNerve(this, &NrvRadiconNpc.HackShake)) {
        al::setNerve(this, &NrvRadiconNpc.WaitHack);
        return true;
    }

    if (rs::isMsgInitCapTarget(msg)) {
        rs::setCapTargetInfo(msg, mCapTargetInfo);
        return true;
    }

    do {
        if (mPlayerHack != nullptr) {
            if (rs::isMsgHackInvalidEscape(msg))
                return _17c;

            if (rs::isMsgHackMarioDead(msg) || rs::isMsgHackMarioDemo(msg) ||
                rs::isMsgHackMarioCheckpointFlagWarp(msg) ||
                (rs::isMsgCancelHack(msg) && !_17c)) {
                al::startVisAnim(this, "HackEnd");
                sead::Vector3f front;
                al::calcQuatFront(&front, mInitQuat);
                front.x = -front.x;
                front.y = -front.y;
                front.z = -front.z;
                sead::Quatf quat = mInitQuat;
                al::rotateQuatYDirDegree(&quat, quat, 180.0f);
                rs::endHackTargetQuat(&mPlayerHack, quat, front);
                mRadiconCar->endHackMove();
                break;
            }
        }

        if (!al::isNerve(this, &NrvRadiconNpc.WaitHack))
            return false;
        if (rs::isMsgCapKeepLockOn(msg))
            return true;
        if (rs::isMsgStartHack(msg)) {
            al::invalidateClipping(this);
            mPlayerHack = rs::startHack(self, other, nullptr);
            rs::startHackStartDemo(mPlayerHack, this);
            al::setNerve(this, &NrvRadiconNpc.StartHack);
            return true;
        }
        if (!rs::isMsgCapCancelLockOn(msg))
            return false;
    } while (false);

    al::setNerve(this, &NrvRadiconNpc.EndHack);
    return true;
}

void RadiconNpc::control() {}

void RadiconNpc::endHack() {
    al::startVisAnim(this, "HackEnd");
    sead::Vector3f front;
    al::calcQuatFront(&front, mInitQuat);
    front.x = -front.x;
    front.y = -front.y;
    front.z = -front.z;
    sead::Quatf quat = mInitQuat;
    al::rotateQuatYDirDegree(&quat, quat, 180.0f);
    rs::endHackTargetQuat(&mPlayerHack, quat, front);
    mRadiconCar->endHackMove();
    al::setNerve(this, &NrvRadiconNpc.EndHack);
}

void RadiconNpc::exeWait() {
    if (al::isFirstStep(this)) {
        al::validateClipping(this);
        al::startAction(this, "RadiConNpcWait");
        rs::startEventFlow(mEventFlowExecutor, "Wait");
    }

    if (al::isCurrentEventEntry(mEventFlowExecutor, "Wait") && mRadiconCar->isBroken())
        rs::startEventFlow(mEventFlowExecutor, "Broken");

    if (al::isCurrentEventEntry(mEventFlowExecutor, "Broken") && !mRadiconCar->isBroken() &&
        !al::isHideModel(mRadiconCar))
        rs::startEventFlow(mEventFlowExecutor, "Wait");

    mRandomActionUpdater->update();
    rs::updateEventFlow(mEventFlowExecutor);
}

void RadiconNpc::exeWaitHack() {}

void RadiconNpc::exeStartHack() {
    if (al::isFirstStep(this)) {
        rs::offRouteGuideByActor(this);
        rs::addDemoSubActor(this);
        rs::hideShadowHackCap(mPlayerHack);
        al::startAction(this, "HackStart");
    }

    if (al::isStep(this, 17))
        mPlayerHackStartShaderCtrl->start();
    mPlayerHackStartShaderCtrl->update();

    if (al::isActionEnd(this)) {
        rs::endHackStartDemo(mPlayerHack, this);
        mPlayerHackStartShaderCtrl->end();
        al::tryOnStageSwitch(this, "SwitchHackKeepOn");
        al::startCamera(this, mCameraTicket, -1);
        mRadiconCar->startHackMove();
        const sead::Vector3f& carTrans = al::getTrans(mRadiconCar);
        mCarTrans.z = carTrans.z;
        *(u64*)&mCarTrans.x = *(u64*)&carTrans.x;
        al::setNerve(this, &NrvRadiconNpc.HackWait);
    }
}

// NON_MATCHING: regswap
void RadiconNpc::exeHackWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "RadiConHackWait");

    const sead::Vector3f& carTrans = al::getTrans(mRadiconCar);
    mCarTrans.z = carTrans.z;
    *(u64*)&mCarTrans.x = *(u64*)&carTrans.x;
    mJointLookAtController->requestJointLookAt(mCarTrans);
    u8* ctrlBytes = reinterpret_cast<u8*>(mJointLookAtController);
    u8 oldVal = ctrlBytes[0x50];
    ctrlBytes[0x50] = 0;
    ctrlBytes[0x51] = oldVal;

    if (rs::isTriggerHackSwing(mPlayerHack)) {
        al::setNerve(this, &NrvRadiconNpc.HackShake);
        return;
    }

    if (mRadiconCar->isBroken()) {
        al::startVisAnim(this, "HackEnd");
        sead::Vector3f front;
        al::calcQuatFront(&front, mInitQuat);
        front.x = -front.x;
        front.y = -front.y;
        front.z = -front.z;
        sead::Quatf quat = mInitQuat;
        al::rotateQuatYDirDegree(&quat, quat, 180.0f);
        rs::endHackTargetQuat(&mPlayerHack, quat, front);
        mRadiconCar->endHackMove();
        al::setNerve(this, &NrvRadiconNpc.EndHack);
    }
}

void RadiconNpc::exeHackShake() {
    if (al::isFirstStep(this))
        al::startAction(this, "RadiConShake");
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvRadiconNpc.HackWait);
}

void RadiconNpc::exeEndHack() {
    if (al::isFirstStep(this)) {
        if (al::isActiveCamera(mCameraTicket))
            al::endCamera(this, mCameraTicket, -1, false);
        al::tryOffStageSwitch(this, "SwitchHackKeepOn");
        al::invalidateClipping(this);
        rs::onRouteGuideByActor(this);
        al::startAction(this, "HackEnd");
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvRadiconNpc.Wait);
}

void RadiconNpc::exeReaction() {
    al::updateNerveStateAndNextNerve(this, &NrvRadiconNpc.Wait);
}
