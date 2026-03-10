#include "Npc/RadiconCar.h"

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Controller/PadRumbleFunction.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Matrix/MatrixUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Se/SeFunction.h"

#include "Enemy/EnemyStateReset.h"
#include "Npc/RadiconCarStateAutoMove.h"
#include "Player/IUsePlayerHack.h"
#include "Player/PlayerHackKeeper.h"
#include "Util/Hack.h"
#include "Util/SensorMsgFunction.h"

namespace {
using namespace al;

NERVE_HOST_TYPE_IMPL(RadiconCar, Wait)

class HostTypeNrvMoveAuto : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        keeper->getParent<RadiconCar>()->exeMoveAuto();
    }

    void executeOnEnd(al::NerveKeeper* keeper) const override {
        keeper->getParent<RadiconCar>()->endMoveAuto();
    }
};

NERVE_HOST_TYPE_IMPL(RadiconCar, Reset)
NERVE_HOST_TYPE_IMPL(RadiconCar, Break)
NERVE_HOST_TYPE_IMPL(RadiconCar, Reaction)
NERVE_HOST_TYPE_IMPL(RadiconCar, Appear)
NERVE_HOST_TYPE_IMPL(RadiconCar, StartHackMove)

class HostTypeNrvMoveRun : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        keeper->getParent<RadiconCar>()->exeMove();
    }

    void executeOnEnd(al::NerveKeeper* keeper) const override {
        keeper->getParent<RadiconCar>()->endMoveRun();
    }
};

NERVE_HOST_TYPE_IMPL_(RadiconCar, MoveStop, Move)

struct {
    HostTypeNrvWait Wait;
    HostTypeNrvMoveAuto MoveAuto;
    HostTypeNrvReset Reset;
    HostTypeNrvBreak Break;
    HostTypeNrvReaction Reaction;
    HostTypeNrvAppear Appear;
    HostTypeNrvStartHackMove StartHackMove;
    HostTypeNrvMoveRun MoveRun;
    HostTypeNrvMoveStop MoveStop;
} NrvRadiconCar;
}  // namespace

RadiconCar::RadiconCar(IUsePlayerHack** hackPtr)
    : al::LiveActor("ラジコンカー"), mPlayerHackPtr(hackPtr) {}

// NON_MATCHING: vtable offset for initAfterPlacement virtual call (0x20 vs 0x10)
void RadiconCar::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "RadiConCar", nullptr);
    al::setEffectFollowMtxPtr(this, "Wait", &mEffectMtx);
    al::setEffectFollowMtxPtr(this, "Move", &mEffectMtx);
    al::initNerve(this, &NrvRadiconCar.Wait, 8);
    mStateAutoMove = new RadiconCarStateAutoMove(this);
    al::initNerveState(this, mStateAutoMove, &NrvRadiconCar.MoveAuto, "状態:自動運転");
    mStateReset = new EnemyStateReset(this, initInfo, nullptr);
    al::initNerveState(this, mStateReset, &NrvRadiconCar.Reset, "リセット");
    al::createAndSetColliderIgnoreOptionalPurpose(this, "RadiConCarThrough");
    al::hideSilhouetteModelIfShow(this);
    al::initJointControllerKeeper(this, 3);
    mJointSpringControllerHolder = new al::JointSpringControllerHolder();
    mJointSpringControllerHolder->init(this, "InitJointSpringCtrl");
    initAfterPlacement();
}

void RadiconCar::initAfterPlacement() {
    mStateReset->mPos = al::getTrans(this);
    sead::Vector3f rotate;
    al::calcQuatRotateDegree(&rotate, al::getQuat(this));
    mStateReset->mRot = rotate;
}

void RadiconCar::makeActorAlive() {
    al::LiveActor::makeActorAlive();
    al::setNerve(this, mIsRace ? (const al::Nerve*)&NrvRadiconCar.Wait : &NrvRadiconCar.MoveAuto);
}

// NON_MATCHING: regalloc differences in tryEmitEffect/tryDeleteEffect calls
void RadiconCar::updateEffect() {
    if (mAirFrameCount >= 5) {
        al::tryDeleteEffect(this, "Wait");
        al::tryDeleteEffect(this, "Move");
        return;
    }

    al::tryEmitEffect(this, al::isNerve(this, &NrvRadiconCar.MoveAuto) ? "Wait" : "Move", nullptr);

    if (mIsRace) {
        const sead::Vector3f& gravity = al::getGravity(this);
        f32 gx = gravity.x;
        f32 gy = gravity.y;
        f32 gz = gravity.z;
        const sead::Vector3f& trans = al::getTrans(this);
        sead::Vector3f start = {trans.x - gx * 100.0f, trans.y - gy * 100.0f,
                                trans.z - gz * 100.0f};
        sead::Vector3f dir = {gx * 500.0f, gy * 500.0f, gz * 500.0f};
        sead::Vector3f hitPos;
        sead::Vector3f hitNormal;
        if (!alCollisionUtil::getHitPosAndNormalOnArrow(this, &hitPos, &hitNormal, start, dir,
                                                        nullptr, nullptr))
            return;
        sead::Vector3f front;
        al::calcFrontDir(&front, this);
        al::makeMtxUpFrontPos(&mEffectMtx, hitNormal, front, hitPos);
    } else {
        sead::Vector3f front;
        al::calcFrontDir(&front, this);
        sead::Vector3f pos = al::getTrans(this);
        pos.y = mStateReset->mPos.y;
        const sead::Vector3f& gravity = al::getGravity(this);
        sead::Vector3f up = {-gravity.x, -gravity.y, -gravity.z};
        al::makeMtxUpFrontPos(&mEffectMtx, up, front, pos);
    }
}

void RadiconCar::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &NrvRadiconCar.Break))
        return;

    if (*mPlayerHackPtr) {
        if (rs::sendMsgPlayerItemGetAll(other, self))
            return;
    } else {
        if (rs::sendMsgCapAttack(other, self))
            return;
    }

    if (al::sendMsgPush(other, self))
        return;
    rs::sendMsgPushToPlayer(other, self);
}

bool RadiconCar::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardTargetMarker(msg))
        return true;
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    if (!al::isNerve(this, &NrvRadiconCar.Appear) && !al::isNerve(this, &NrvRadiconCar.Break) &&
        !al::isNerve(this, &NrvRadiconCar.Reset)) {
        if (al::tryReceiveMsgPushAndAddVelocityH(this, msg, other, self, 4.0f))
            return true;
    }

    if (rs::isMsgPlayerAndCapObjHipDropAll(msg) && !al::isNerve(this, &NrvRadiconCar.Reset) &&
        !al::isNerve(this, &NrvRadiconCar.Break) &&
        !(al::isNerve(this, &NrvRadiconCar.Reaction) && al::isLessStep(this, 10))) {
        al::setVelocityZero(this);
        rs::requestHitReactionToAttacker(msg, self, other);
        al::setNerve(this, &NrvRadiconCar.Break);
        return false;
    }

    if (al::isNerve(this, &NrvRadiconCar.Reset))
        return false;
    if (al::isNerve(this, &NrvRadiconCar.Break))
        return false;
    if (al::isNerve(this, &NrvRadiconCar.Reaction) && al::isLessStep(this, 10))
        return false;

    bool isHit = false;

    if (rs::isMsgCapAttack(msg) || al::isMsgPlayerTrampleReflect(msg) ||
        al::isMsgPlayerTrample(msg)) {
        al::LiveActor* attacker = al::getSensorHost(other);
        if (al::getVelocity(attacker).y < -10.0f)
            isHit = true;
    }

    if (!isHit) {
        if (rs::isMsgCapAttack(msg) || rs::isMsgSeedTouch(msg) || rs::isMsgSeedReflect(msg) ||
            rs::isMsgMayorItemReflect(msg))
            isHit = true;
    }

    if (isHit) {
        al::setVelocityZero(this);
        al::startHitReaction(this, "踏まれ");
        rs::requestHitReactionToAttacker(msg, self, other);
        al::setNerve(this, &NrvRadiconCar.Reaction);
        return true;
    }
    return false;
}

void RadiconCar::setTypeRace() {
    mIsRace = true;
    al::setNerve(this, &NrvRadiconCar.Wait);
}

void RadiconCar::startHackMove() {
    if (al::isNerve(this, &NrvRadiconCar.Reset))
        al::setNerve(this, &NrvRadiconCar.Appear);
    else
        al::setNerve(this, &NrvRadiconCar.StartHackMove);
}

void RadiconCar::endHackMove() {
    mIsHacking = false;
    alPadRumbleFunction::stopPadRumbleLoop(this, "【ループ】ブーン（中）", al::getTransPtr(this),
                                           -1);
    al::tryDeleteEffect(this, "Move");
    al::hideSilhouetteModelIfShow(this);
    if (!al::isNerve(this, &NrvRadiconCar.Break))
        al::setNerve(this,
                     mIsRace ? (const al::Nerve*)&NrvRadiconCar.Wait : &NrvRadiconCar.MoveAuto);
}

bool RadiconCar::isMoving() const {
    return al::isNerve(this, &NrvRadiconCar.MoveRun) || al::isNerve(this, &NrvRadiconCar.MoveStop);
}

bool RadiconCar::isBroken() const {
    return al::isNerve(this, &NrvRadiconCar.Reset) && al::isGreaterStep(this, 45);
}

void RadiconCar::tryCancelHack() const {
    IUsePlayerHack* player = *mPlayerHackPtr;
    if (!player)
        return;
    al::LiveActor* hackActor = player->getPlayerHackKeeper()->getHack();
    al::HitSensor* playerBody = al::getHitSensor(hackActor, "Body");
    al::HitSensor* selfBody = al::getHitSensor(this, "Body");
    rs::sendMsgCancelHack(playerBody, selfBody);
}

void RadiconCar::updateVelocity() {
    if (al::isOnGround(this, 0))
        mAirFrameCount = 0;
    else
        mAirFrameCount++;

    if (al::isOnGround(this, 0)) {
        sead::Vector3f front;
        sead::Vector3f side;
        al::calcFrontDir(&front, this);
        al::calcSideDir(&side, this);
        al::scaleVelocityDirection(this, front, 0.972f);
        al::scaleVelocityDirection(this, side, 0.88f);
        al::addVelocityToGravityFittedGround(this, 1.5f, 0);
    } else {
        al::scaleVelocity(this, 0.97f);
        al::addVelocityToGravity(this, 1.5f);
    }

    if (mIsRace)
        al::reboundVelocityFromEachCollision(this, 0.0f, 0.0f, 0.0f, 0.0f);
    else if (al::reboundVelocityFromEachCollision(this, 0.0f, 0.15f, 0.0f, 0.0f))
        al::scaleVelocityY(this, 0.6f);
}

void RadiconCar::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");
    if (al::isVelocitySlowH(this, 1.0f) && al::isInvalidClipping(this))
        al::validateClipping(this);
    updateVelocity();
}

// NON_MATCHING: fmin instruction difference (fmin vs fminnm) and regalloc
void RadiconCar::calcAccel(sead::Vector3f* accel, bool isOnGround) {
    sead::Vector3f front;
    sead::Vector3f up;
    al::calcFrontDir(&front, this);
    al::calcUpDir(&up, this);

    sead::Vector3f groundNormal;
    if (al::isOnGround(this, 0)) {
        sead::Vector3f colliderPos;
        al::calcColliderPos(&colliderPos, this);
        f32 frontX = front.x * 55.0f;
        f32 frontY = front.y * 55.0f;
        f32 frontZ = front.z * 55.0f;

        const sead::Vector3f& velocity = al::getVelocity(this);
        f32 dot = front.x * velocity.x + front.y * velocity.y + front.z * velocity.z;
        sead::Vector3f rayStart;
        if (dot >= 0.0f)
            rayStart = {frontX + colliderPos.x, frontY + colliderPos.y, frontZ + colliderPos.z};
        else
            rayStart = {colliderPos.x - frontX, colliderPos.y - frontY, colliderPos.z - frontZ};

        f32 radius = al::getColliderRadius(this);
        sead::Vector3f rayDir = {up.x * radius * -2.0f, up.y * radius * -2.0f,
                                 up.z * radius * -2.0f};

        sead::Vector3f hitPos;
        sead::Vector3f hitNormal;
        if (alCollisionUtil::getHitPosAndNormalOnArrow(this, &hitPos, &hitNormal, rayStart, rayDir,
                                                       nullptr, nullptr))
            groundNormal = hitNormal;
        else
            groundNormal = al::getOnGroundNormal(this, 0);
    } else {
        const sead::Vector3f& gravity = al::getGravity(this);
        groundNormal = {-gravity.x, -gravity.y, -gravity.z};
    }

    al::convergeVec(&up, up, groundNormal, 0.1f);
    al::makeQuatUpFront(al::getQuatPtr(this), up, front);

    sead::Vector3f newFront;
    al::calcFrontDir(&newFront, this);

    sead::Vector3f projFront;
    if (al::isOnGround(this, 0)) {
        const sead::Vector3f& velocity = al::getVelocity(this);
        f32 forwardSpeed =
            newFront.x * velocity.x + newFront.y * velocity.y + newFront.z * velocity.z;
        sead::Vector3f colliderPos;
        al::calcColliderPos(&colliderPos, this);
        f32 sign = 1.0f;
        if (forwardSpeed < 0.0f)
            sign = -1.0f;

        sead::Vector3f rayStart = {newFront.x * sign * 55.0f + colliderPos.x,
                                   newFront.y * sign * 55.0f + colliderPos.y,
                                   newFront.z * sign * 55.0f + colliderPos.z};
        sead::Vector3f upDir;
        al::calcUpDir(&upDir, this);
        f32 radius = al::getColliderRadius(this);
        sead::Vector3f rayDir = {upDir.x * radius * -2.0f, upDir.y * radius * -2.0f,
                                 upDir.z * radius * -2.0f};

        sead::Vector3f hitNormal2;
        sead::Vector3f hitPos2;
        if (alCollisionUtil::getHitPosAndNormalOnArrow(this, &hitNormal2, &hitPos2, rayStart,
                                                       rayDir, nullptr, nullptr)) {
            sead::Quatf tempQuat;
            sead::Vector3f sideDir;
            al::calcSideDir(&sideDir, this);
            al::makeQuatUpSide(&tempQuat, hitPos2, sideDir);
            al::calcQuatFront(&projFront, tempQuat);
        } else {
            projFront = al::getOnGroundNormal(this, 0);
            // recalculate with ground normal
            sead::Quatf tempQuat;
            sead::Vector3f sideDir;
            al::calcSideDir(&sideDir, this);
            al::makeQuatUpSide(&tempQuat, projFront, sideDir);
            al::calcQuatFront(&projFront, tempQuat);
        }
    } else {
        projFront = newFront;
    }

    f32 throttle = 0.0f;
    if (mIsHacking) {
        IUsePlayerHack* player = *mPlayerHackPtr;
        if (player)
            throttle = rs::getRadiconInputThrottleValue(player);
    }

    if (isOnGround) {
        f32 steering = 0.0f;
        if (mIsHacking) {
            IUsePlayerHack* player = *mPlayerHackPtr;
            if (player)
                steering = rs::getRadiconInputSteeringValue(player);
        }
        if (al::isNearZero(throttle, 0.001f) && !al::isNearZero(steering, 0.001f)) {
            const sead::Vector3f& velocity = al::getVelocity(this);
            f32 forwardDot =
                projFront.x * velocity.x + projFront.y * velocity.y + projFront.z * velocity.z;
            throttle = al::sign(forwardDot) * 0.01f;
        }
        f32 speed = al::calcSpeed(this);
        if (speed <= 0.0f)
            speed = -speed;
        al::rotateQuatYDirDegree(this, speed * (steering * al::sign(throttle) * -0.14f));
    }

    accel->x = throttle * projFront.x * 1.5f;
    accel->y = throttle * projFront.y * 1.5f;
    accel->z = throttle * projFront.z * 1.5f;
}

f32 RadiconCar::calcThrottleInputValue() const {
    if (!mIsHacking)
        return 0.0f;
    IUsePlayerHack* player = *mPlayerHackPtr;
    if (!player)
        return 0.0f;
    return rs::getRadiconInputThrottleValue(player);
}

f32 RadiconCar::calcSteeringInputValue() const {
    if (!mIsHacking)
        return 0.0f;
    IUsePlayerHack* player = *mPlayerHackPtr;
    if (!player)
        return 0.0f;
    return rs::getRadiconInputSteeringValue(player);
}

// NON_MATCHING: regalloc and scheduling differences in complex movement logic
void RadiconCar::exeMove() {
    if (al::isVelocityFastH(this, 3.0f))
        al::holdSeWithParam(this, "Move", al::calcSpeedH(this), "");

    if (al::isOnGround(this, 4)) {
        sead::Vector3f accel = sead::Vector3f::zero;
        calcAccel(&accel, al::isVelocityFastH(this, 0.1f));
        al::addVelocity(this, accel);
    }
    updateEffect();
    updateVelocity();

    if (al::isNerve(this, &NrvRadiconCar.MoveRun)) {
        if (al::isFirstStep(this)) {
            alPadRumbleFunction::startPadRumbleLoopControlable(this, "【ループ】ブーン（中）",
                                                               al::getTransPtr(this), -1);
        }
        f32 speed = al::calcSpeedH(this);
        f32 ratio = speed / 30.0f;
        f32 volume = fminf(ratio * ratio, 1.0f) * 0.18f;
        f32 pitch = ratio * 0.85f + 0.75f;
        alPadRumbleFunction::changePadRumbleLoopVolmue(this, "【ループ】ブーン（中）",
                                                       al::getTransPtr(this), volume, volume, -1);
        alPadRumbleFunction::changePadRumbleLoopPitch(this, "【ループ】ブーン（中）",
                                                      al::getTransPtr(this), pitch, pitch, -1);
        if (al::isCollidedWallFace(this)) {
            if (al::isVelocityFastH(this, 10.0f))
                al::startHitReaction(this, "壁ヒット(強)");
            else if (al::isVelocityFastH(this, 3.0f))
                al::startHitReaction(this, "壁ヒット(弱)");
        }
        if (al::isVelocitySlowH(this, 1.0f)) {
            al::setNerve(this, &NrvRadiconCar.MoveStop);
            return;
        }
    }
    if (al::isNerve(this, &NrvRadiconCar.MoveStop) && al::isVelocityFastH(this, 3.0f)) {
        al::setNerve(this, &NrvRadiconCar.MoveRun);
        return;
    }

    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, this);
    sead::Vector3f sideDir;
    al::calcSideDir(&sideDir, this);
    if (al::isCollidedWallFace(this)) {
        const sead::Vector3f& wallNormal = al::getCollidedWallNormal(this);
        const sead::Vector3f& gravity = al::getGravity(this);
        f32 cross = frontDir.x * (wallNormal.z * gravity.y - wallNormal.y * gravity.z) +
                    frontDir.y * (wallNormal.x * gravity.z - wallNormal.z * gravity.x) +
                    frontDir.z * (wallNormal.y * gravity.x - wallNormal.x * gravity.y);
        f32 s = al::sign(cross);
        al::rotateQuatYDirDegree(this, s + s);
    }
}

void RadiconCar::endMoveRun() {
    alPadRumbleFunction::stopPadRumbleLoop(this, "【ループ】ブーン（中）", al::getTransPtr(this),
                                           -1);
}

void RadiconCar::exeMoveAuto() {
    al::updateNerveState(this);
    updateEffect();
}

void RadiconCar::endMoveAuto() {
    al::tryDeleteEffect(this, "Wait");
}

void RadiconCar::exeReaction() {
    if (al::isFirstStep(this))
        al::startAction(this, "Reaction");
    if (al::isActionEnd(this)) {
        if (*mPlayerHackPtr)
            al::setNerve(this, &NrvRadiconCar.MoveStop);
        else
            al::setNerve(this,
                         mIsRace ? (const al::Nerve*)&NrvRadiconCar.Wait : &NrvRadiconCar.MoveAuto);
    }
}

void RadiconCar::exeBreak() {
    if (al::isFirstStep(this)) {
        al::setVelocityZero(this);
        al::startAction(this, "Disappear");
        al::startHitReaction(this, "消滅");
        al::invalidateClipping(this);
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvRadiconCar.Reset);
}

void RadiconCar::exeReset() {
    al::updateNerveStateAndNextNerve(this, mIsRace ? (const al::Nerve*)&NrvRadiconCar.Wait :
                                                     &NrvRadiconCar.MoveAuto);
}

void RadiconCar::exeAppear() {
    al::startHitReaction(this, "出現");
    al::setNerve(this, &NrvRadiconCar.StartHackMove);
}

void RadiconCar::exeStartHackMove() {
    mIsHacking = !mIsRace;
    al::invalidateClipping(this);
    al::showSilhouetteModelIfHide(this);
    al::startAction(this, "Move");
    al::tryDeleteEffect(this, "Wait");
    al::setNerve(this, &NrvRadiconCar.MoveStop);
}
