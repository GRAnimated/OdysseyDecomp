#include "Npc/YukimaruRacer.h"

#include <cmath>
#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Area/AreaObjUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Rail/RailUtil.h"

#include "Npc/SnowManRaceFunction.h"
#include "Npc/YukimaruStateMove.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(YukimaruRacer, Wait);
NERVE_IMPL(YukimaruRacer, Run);
NERVES_MAKE_NOSTRUCT(YukimaruRacer, Wait, Run);
}  // namespace

YukimaruRacer::YukimaruRacer(const char* name) : al::LiveActor(name) {
    mRotation.set(sead::Quatf::unit);
    mStateMove = nullptr;
    mMoveVec.set(sead::Vector3f::zero);
    _134 = false;
}

// NON_MATCHING: vtable offset differences at appear/kill (0x20/0x30 vs 0x18/0x28)
void YukimaruRacer::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "SnowManRacer", "Racer");

    mRotation = al::getQuat(this);
    al::initJointControllerKeeper(this, 1);
    al::initJointGlobalQuatController(this, &mRotation, "Rotate");

    al::initNerve(this, &Wait, 1);

    mStateMove = new YukimaruStateMove(this, this, &mRotation);
    al::initNerveState(this, mStateMove, &Run, u8"レース状態");

    al::setRailPosToNearestPos(this, al::getTrans(this));

    const TalkNpcParam* talkParam = rs::initTalkNpcParam(this, nullptr);
    if (!rs::tryApplyNpcMaterialAnimPresetFromPlacementInfo(this, initInfo, talkParam))
        rs::setNpcMaterialAnimFromPlacementInfo(this, initInfo);

    if (al::isExistRail(this)) {
        al::tryGetArg(&_134, initInfo, "IsDemoRun");
        if (_134)
            al::setNerve(this, &Run);
        makeActorAlive();
    } else {
        makeActorDead();
    }
}

void YukimaruRacer::initAfterPlacement() {
    if (!_134)
        SnowManRaceFunction::registerNpcToRaceWatcher(this);
}

void YukimaruRacer::movement() {
    al::updateMaterialCodeAll(this);
    al::LiveActor::movement();
}

void YukimaruRacer::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &Run))
        mStateMove->attackSensor(self, other);
}

bool YukimaruRacer::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                               al::HitSensor* self) {
    if (rs::isMsgRaceStart(msg)) {
        if (al::isNerve(this, &Wait))
            al::setNerve(this, &Run);
        return true;
    }

    if (al::isNerve(this, &Run))
        mStateMove->receiveMsg(msg, other, self);
    return false;
}

void YukimaruRacer::start() {
    al::setNerve(this, &Run);
}

void YukimaruRacer::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "ReadyEnemy");
    YukimaruMovement::updateVelocity(this);
}

void YukimaruRacer::exeRun() {
    updateMoveVec();
    al::updateNerveState(this);
}

// NON_MATCHING: fmax vs fmaxnm instruction; register allocation uses x23/x24;
// branch conditions b.pl vs b.ge; sead::Vector3f operator= at mMoveVec assignment
void YukimaruRacer::updateMoveVec() {
    const sead::Vector3f& gravity = al::getGravity(this);

    sead::Vector3f velocity = al::getVelocity(this);
    al::verticalizeVec(&velocity, gravity, velocity);

    sead::Vector3f railDir = al::getRailDir(this);
    al::verticalizeVec(&railDir, gravity, railDir);
    al::tryNormalizeOrZero(&railDir);

    const sead::Vector3f& railPos = al::getRailPos(this);
    const sead::Vector3f& trans = al::getTrans(this);

    sead::Vector3f diff = railPos - trans;
    al::verticalizeVec(&diff, gravity, diff);

    f32 dot = railDir.dot(diff);

    f32 speed;
    if (dot >= 1500.0f) {
        speed = 2.0f;
    } else {
        speed = fmaxf(1500.0f - dot, 2.0f);

        f32 velMag = velocity.length();
        if (velMag >= speed) {
            speed = velMag;
            if (velMag > 200.0f)
                speed = 200.0f;
        }
    }

    al::moveRail(this, speed);

    const sead::Vector3f& newRailPos = al::getRailPos(this);
    const sead::Vector3f& newTrans = al::getTrans(this);

    sead::Vector3f target = newRailPos - (newTrans + velocity);
    al::verticalizeVec(&target, gravity, target);

    sead::Vector3f dir;
    if (!al::tryNormalizeOrZero(&dir, target))
        dir = al::getRailDir(this);

    mMoveVec = dir;
}

bool YukimaruRacer::isTriggerJump() const {
    if (al::isInAreaObj(this, "YukimaruRacerNoJumpArea", al::getTrans(this)))
        return false;
    return al::isOnGround(this, 0);
}

bool YukimaruRacer::isHoldJump() const {
    return al::isInAreaObj(this, "YukimaruRacerHoldJumpArea", al::getTrans(this));
}

void YukimaruRacer::calcInputVec(sead::Vector3f* out) const {
    out->set(mMoveVec);
}
