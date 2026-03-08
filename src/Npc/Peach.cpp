#include "Npc/Peach.h"

#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Util/NpcAnimUtil.h"

namespace {
NERVE_IMPL(Peach, Wait)
NERVE_IMPL(Peach, WaitKoopaShip)
NERVE_IMPL(Peach, WaitKoopaShipWithKoopa)
NERVE_IMPL(Peach, WaitChurch)
NERVE_END_IMPL(Peach, WaitDemo)
NERVES_MAKE_NOSTRUCT(Peach, Wait, WaitKoopaShip, WaitKoopaShipWithKoopa, WaitChurch, WaitDemo)
}  // namespace

Peach::Peach(const char* name) : al::LiveActor(name) {}

void Peach::init(const al::ActorInitInfo& initInfo) {
    al::initActorChangeModel(this, initInfo);
    al::initNerve(this, &Wait, 0);
    al::startVisAnim(this, "TiaraOn");
    mJointSpringHolder = al::JointSpringControllerHolder::tryCreateAndInitJointControllerKeeper(
        this, "InitJointSpringCtrl");
    al::LiveActor* bouquet = al::tryGetSubActor(this, u8"ピーチブーケ");
    if (bouquet)
        mSubActorSpringHolder =
            al::JointSpringControllerHolder::tryCreateAndInitJointControllerKeeper(
                bouquet, "InitJointSpringCtrl");
    makeActorAlive();
}

void Peach::control() {
    rs::trySwitchDepthToSelfShadow(this);
}

void Peach::makeActorDeadBouquet() {
    al::LiveActor* bouquet = al::tryGetSubActor(this, u8"ピーチブーケ");
    if (bouquet) {
        al::offSyncAppearSubActor(this, u8"ピーチブーケ");
        bouquet->makeActorDead();
    }
}

void Peach::startWaitKoopaShip() {
    al::setNerve(this, &WaitKoopaShip);
}

void Peach::startWaitKoopaShipWithKoopa() {
    al::setNerve(this, &WaitKoopaShipWithKoopa);
}

void Peach::startWaitChurch() {
    al::LiveActor* bouquet = al::tryGetSubActor(this, u8"ピーチブーケ");
    if (bouquet) {
        al::offSyncAppearSubActor(this, u8"ピーチブーケ");
        bouquet->makeActorDead();
    }
    al::setNerve(this, &WaitChurch);
}

void Peach::startWaitDemo(bool disableSpring) {
    if (al::isNerve(this, &WaitDemo))
        return;

    if (al::isVisAnimPlaying(this)) {
        mPreviousVisAnimName = al::getPlayingVisAnimName(this);
        al::startVisAnim(this, "TiaraDemo");
    } else {
        mPreviousVisAnimName = nullptr;
    }

    if (disableSpring)
        mJointSpringHolder->offControlAll();
    else
        mJointSpringHolder->onControlAll();

    al::LiveActor* bouquet = al::tryGetSubActor(this, u8"ピーチブーケ");
    if (bouquet) {
        reinterpret_cast<u8*>(bouquet)[0x142] = 0;
        al::copyPose(bouquet, this);
        if (disableSpring)
            mSubActorSpringHolder->offControlAll();
        else
            mSubActorSpringHolder->onControlAll();
    }
    al::setNerve(this, &WaitDemo);
}

void Peach::startDemoAction(const char* actionName) {
    rs::startNpcAction(this, actionName);
    syncEyeSubActorAction("Wait");
    al::LiveActor* bouquet = al::tryGetSubActor(this, u8"ピーチブーケ");
    if (bouquet) {
        if (al::isExistAction(bouquet, actionName))
            al::startAction(bouquet, actionName);
    }
}

void Peach::syncEyeSubActorAction(const char* fallbackAction) {
    al::LiveActor* eyeActor = al::tryGetSubActor(this, u8"ティアラの目");
    if (!eyeActor)
        return;
    if (al::isExistAction(eyeActor, al::getActionName(this)))
        al::startAction(eyeActor, al::getActionName(this));
    else if (al::isExistAction(eyeActor, fallbackAction))
        al::startAction(eyeActor, fallbackAction);
}

void Peach::exeWait() {
    if (al::isFirstStep(this)) {
        rs::startNpcAction(this, "Wait");
        syncEyeSubActorAction("Wait");
    }
}

void Peach::exeWaitKoopaShip() {
    if (al::isFirstStep(this)) {
        rs::startNpcAction(this, "Wait");
        al::startVisAnim(this, "TiaraOn");
        syncEyeSubActorAction("Wait");
    }
}

void Peach::exeWaitKoopaShipWithKoopa() {
    if (al::isFirstStep(this)) {
        rs::startNpcAction(this, "KoopaShipWait");
        syncEyeSubActorAction("KoopaShipWait");
    }
}

void Peach::exeWaitChurch() {
    if (al::isFirstStep(this)) {
        rs::startNpcAction(this, "WaitChurch");
        al::LiveActor* eyeActor = al::tryGetSubActor(this, u8"ティアラの目");
        if (eyeActor)
            al::startAction(eyeActor, "Wait");
    }
}

void Peach::exeWaitDemo() {}

void Peach::endWaitDemo() {
    if (!mPreviousVisAnimName)
        return;
    al::startVisAnim(this, mPreviousVisAnimName);
    mPreviousVisAnimName = nullptr;
    mJointSpringHolder->onControlAll();
    al::LiveActor* bouquet = al::tryGetSubActor(this, u8"ピーチブーケ");
    if (bouquet) {
        al::startAction(bouquet, "Wait");
        reinterpret_cast<u8*>(bouquet)[0x142] = 1;
        mSubActorSpringHolder->onControlAll();
    }
}
