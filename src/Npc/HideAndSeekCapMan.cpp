#include "Npc/HideAndSeekCapMan.h"

#include <math/seadVector.h>

#include "Library/Camera/CameraUtil.h"
#include "Library/Event/EventFlowExecutor.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"

namespace {
NERVE_IMPL(HideAndSeekCapMan, Hide)
NERVE_IMPL(HideAndSeekCapMan, Appear)
NERVE_IMPL(HideAndSeekCapMan, Wait)
NERVE_IMPL(HideAndSeekCapMan, Scared)
NERVE_IMPL(HideAndSeekCapMan, Disappear)
NERVES_MAKE_NOSTRUCT(HideAndSeekCapMan, Hide, Appear, Wait, Scared, Disappear)

const sead::Vector3f sBalloonOffset(0.0f, 15.0f, 0.0f);
}  // namespace

HideAndSeekCapMan::HideAndSeekCapMan(const char* name) : al::PartsModel(name) {}

HideAndSeekCapMan* HideAndSeekCapMan::tryCreateLinkNpcIfClearGame(
    al::LiveActor* parent, const al::ActorInitInfo& info, const char* suffix,
    const char* linkName) {
    if (!al::isExistLinkChild(info, "HideAndSeekCapMan", 0))
        return nullptr;

    auto* actor = new HideAndSeekCapMan(
        al::getLinksActorDisplayName(info, "HideAndSeekCapMan", 0));
    actor->initPartsFixFile(parent, info, "CapManEyes", suffix, linkName);
    al::initLinksActor(actor, info, "HideAndSeekCapMan", 0);
    return actor;
}

void HideAndSeekCapMan::init(const al::ActorInitInfo& initInfo) {
    al::initNerve(this, &Hide, 0);
    mEventFlowExecutor = rs::initEventFlowFromPlacementInfo(this, initInfo, nullptr);
    if (rs::isDefinedEventCamera(mEventFlowExecutor, "Default"))
        rs::initEventCameraObject(mEventFlowExecutor, initInfo, "Default");
    al::setBalloonLocalOffset(mEventFlowExecutor, sBalloonOffset);
    rs::startEventFlow(mEventFlowExecutor, "Init");
    mShineIdx = GameDataFunction::tryFindLinkedShineIndex(this, initInfo);
}

// NON_MATCHING: compiler uses x21 register for accessor instead of stack spill
void HideAndSeekCapMan::control() {
    GameDataHolderAccessor accessor(this);
    GameDataFunction::setHintTrans(accessor, mShineIdx, al::getTrans(this));
}

void HideAndSeekCapMan::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &Wait) || al::isNerve(this, &Scared))
        rs::sendMsgEventFlowScareCheck(other, self, mEventFlowExecutor);
}

bool HideAndSeekCapMan::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                   al::HitSensor* self) {
    return rs::isMsgPlayerDisregardHomingAttack(msg);
}

void HideAndSeekCapMan::exeAppear() {
    if (al::isFirstStep(this))
        al::startAction(this, "Appear");
    if (al::isActionEnd(this)) {
        al::validateHitSensors(this);
        al::setNerve(this, &Wait);
    }
}

void HideAndSeekCapMan::exeWait() {
    if (al::isFirstStep(this))
        rs::startEventFlow(mEventFlowExecutor, "Init");
    rs::updateEventFlow(mEventFlowExecutor);
    if (al::isScare(mEventFlowExecutor)) {
        al::setNerve(this, &Scared);
        return;
    }
    if (rs::isActiveEventDemo(this))
        return;

    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    sead::Vector3f camDir(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&front, this);
    al::calcCameraDir(&camDir, this, 0);

    f32 dot = camDir.x * front.x + camDir.y * front.y + camDir.z * front.z;
    if (dot >= 0.0f) {
        mCounter = 30;
    } else {
        if (--mCounter <= 0)
            al::setNerve(this, &Disappear);
    }
}

void HideAndSeekCapMan::exeDisappear() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Disappear");
        al::invalidateHitSensors(this);
    }
    al::setNerveAtActionEnd(this, &Hide);
}

// NON_MATCHING: b.ge vs b.pl after fcmp
void HideAndSeekCapMan::exeHide() {
    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    sead::Vector3f camDir(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&front, this);
    al::calcCameraDir(&camDir, this, 0);

    f32 dot = camDir.x * front.x + camDir.y * front.y + camDir.z * front.z;
    if (dot >= 0.0f) {
        if (mCounter++ >= 29)
            al::setNerve(this, &Appear);
    } else {
        mCounter = 0;
    }
}

void HideAndSeekCapMan::exeScared() {
    if (al::isFirstStep(this))
        al::startAction(this, "Scared");
    rs::updateEventFlow(mEventFlowExecutor);
    if (!al::isScare(mEventFlowExecutor))
        al::setNerve(this, &Wait);
}
