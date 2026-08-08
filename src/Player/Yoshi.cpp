#include "Player/Yoshi.h"

#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerAnimator.h"
#include "Player/PlayerColliderYoshi.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerEffect.h"
#include "Player/PlayerFunction.h"
#include "Player/PlayerJumpMessageRequest.h"
#include "Player/PlayerModelChangerYoshi.h"
#include "Player/PlayerModelHolder.h"
#include "Player/PlayerPushReceiver.h"
#include "Player/PlayerTrigger.h"
#include "Player/YoshiJointControlKeeper.h"
#include "Player/YoshiStateEgg.h"
#include "Player/YoshiStateHack.h"
#include "Player/YoshiStateNpc.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackFunction.h"
#include "Util/SensorMsgFunction.h"
#include "Util/YoshiUtil.h"

namespace {
NERVE_IMPL(Yoshi, Egg);
NERVE_IMPL(Yoshi, Npc);
NERVE_IMPL(Yoshi, Hack);
NERVES_MAKE_STRUCT(Yoshi, Egg, Npc, Hack);
inline void clampGroundPullSpeed(f32* currentPullSpeed, f32 pullSpeed, f32 horizontalSpeed,
                                 f32 pullRate) {
    f32 maxPullSpeed = fmaxf(horizontalSpeed, 40.0f) * pullRate;
    if (pullSpeed < *currentPullSpeed)
        return;
    *currentPullSpeed = pullSpeed;
    if (!(pullSpeed > maxPullSpeed))
        return;
    *currentPullSpeed = maxPullSpeed;
}
}  // namespace

Yoshi::Yoshi(const char* actorName) : al::LiveActor(actorName) {}

void Yoshi::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "PlayerActorYoshi", nullptr);
    al::onCollide(this);

    auto* modelHolder = new PlayerModelHolder(1);
    auto* modelActor = new al::LiveActor("ヨッシーモデル");
    PlayerFunction::initYoshiModelActor(modelActor, info, "Yoshi");
    modelHolder->registerModel(modelActor, "Normal");
    modelHolder->setModelSuffix("Yoshi");
    modelHolder->changeModel("Normal");
    mModelHolder = modelHolder;

    mPlayerConst = PlayerFunction::createYoshiConst(this, "PlayerActorYoshi", nullptr);
    auto* collider = new PlayerColliderYoshi(this);
    collider->init();
    mCollider = collider;
    mModelChanger = new PlayerModelChangerYoshi(this, mModelHolder);
    mAnimator = new PlayerAnimator(
        mModelHolder, PlayerFunction::createPlayerDitherAnimator(this, 200.0f));
    mEffect = new PlayerEffect(this, mModelHolder, nullptr);
    mTrigger = new PlayerTrigger();
    mJumpMessageRequest = new PlayerJumpMessageRequest();
    mPushReceiver = new PlayerPushReceiver(this);
    mWaterSurfaceFinder = new al::WaterSurfaceFinder(this);
    mCapTargetInfo = rs::createCapTargetInfoWithPlayerCollider(
        mModelHolder->getCurrentModelActor(), mCollider, nullptr);

    al::initNerve(this, &NrvYoshi.Egg, 3);
    mStateEgg = new YoshiStateEgg(info, this, mCollider, mPlayerConst, mWaterSurfaceFinder,
                                 mModelChanger);
    mStateNpc = new YoshiStateNpc(
        this, mPlayerConst, mCollider, mTrigger, mWaterSurfaceFinder, info, mAnimator,
        rs::initEventFlowForSystem(mModelHolder->getCurrentModelActor(), info, nullptr, nullptr,
                                   nullptr));
    mStateHack = new YoshiStateHack(info, this, mModelHolder->getCurrentModelActor(),
                                    mCapTargetInfo, mPlayerConst, mWaterSurfaceFinder, mTrigger,
                                    mModelChanger, mAnimator, mCollider, mEffect,
                                    mJumpMessageRequest);
    al::initNerveState(this, mStateEgg, &NrvYoshi.Egg, "卵");
    al::initNerveState(this, mStateNpc, &NrvYoshi.Npc, "Npc");
    al::initNerveState(this, mStateHack, &NrvYoshi.Hack, "Hack");

    mHitSensor = al::getHitSensor(this, "Body");
    mJointControlKeeper =
        new YoshiJointControlKeeper(mModelHolder->getCurrentModelActor(), mCollider);
    makeActorAlive();
    al::makeMtxRT(&mLocalMtx, this);
}

void Yoshi::initAfterPlacement() {
    rs::noticeCurrentHackYoshi(this);
}

void Yoshi::movement() {
    mTrigger->clearPreMovementTrigger();
    mTrigger->clearReceiveSensorTrigger();
    mTrigger->clearActionTrigger();
    mTrigger->clearAttackSensorTrigger();
    mAnimator->updateAnimFrame();
    mStateHack->updatePrevMovement();
    al::LiveActor::movement();
    mModelChanger->syncHost();
    mPushReceiver->clear();
    mStateHack->updateAfterMovement();

    auto* collider = mCollider;
    const auto* heightCheck = static_cast<const IUsePlayerHeightCheck*>(collider);
    auto* playerConst = mPlayerConst;
    auto* modelHolder = mModelHolder;
    auto* waterSurfaceFinder = mWaterSurfaceFinder;

    const char* materialCode;
    if (rs::isCollidedGround(collider))
        materialCode = rs::getMaterialCodeGround(collider);
    else if (rs::isCollidedWall(collider))
        materialCode = rs::getMaterialCodeWall(collider);
    else if (al::isInFire(this))
        materialCode = "LavaWhite";
    else
        materialCode = "NoCollide";

    const bool isPuddle = rs::isInPuddleHeight(waterSurfaceFinder, heightCheck,
                                               playerConst->getSwimCenterOffset());
    const bool isWet = al::isInAreaObj(this, "WetArea") && !isPuddle;
    al::updateMaterialCodeWet(this, isWet);
    al::updateMaterialCodePuddle(this, isPuddle);
    al::setMaterialCode(this, materialCode);

    auto* modelActor = modelHolder->getCurrentModelActor();
    al::updateMaterialCodeWet(modelActor, isWet);
    al::updateMaterialCodePuddle(modelActor, isPuddle);
    al::setMaterialCode(modelActor, materialCode);

    sead::Vector3f lookAtPosition = {0.0f, 0.0f, 0.0f};
    auto* stateNpc = mStateNpc;
    auto* stateHack = mStateHack;
    bool hasLookAtPosition = false;
    if (al::isNerve(this, &NrvYoshi.Npc))
        hasLookAtPosition = stateNpc->tryGetLookAtPlayerPos(&lookAtPosition);
    else if (al::isNerve(this, &NrvYoshi.Hack))
        hasLookAtPosition = stateHack->tryGetLookAtTonguePos(&lookAtPosition);

    auto** jointControlKeeper = &mJointControlKeeper;
    if (hasLookAtPosition) {
        (*jointControlKeeper)->setLookAtActive(true);
        (*jointControlKeeper)->setLookAtPosition(lookAtPosition);
    } else {
        (*jointControlKeeper)->setLookAtActive(false);
    }

    bool isHeadCorrectionActive = mStateHack->isActiveHeadCorrection();
    (*jointControlKeeper)->setHeadCorrectionActive(isHeadCorrectionActive);
    f32 frontRate = 0.0f;
    f32 sideRate = 0.0f;
    mStateHack->calcGroundPoseRate(&frontRate, &sideRate);
    (*jointControlKeeper)->setGroundPoseRate(frontRate, sideRate);
    (*jointControlKeeper)->update();
}

void Yoshi::control() {
    mAnimator->updateModel();
    if (mAnimator->isSubAnimPlaying() && mAnimator->isSubAnimEnd())
        mAnimator->endSubAnim();
}

// NON_MATCHING: exact 1768-byte size; commutative FMUL operands are reversed at target +0x49485C. Next hypothesis is local lifetime/order refinement around the collider scale multiply.
void Yoshi::updateCollider() {
    al::updatePoseTrans(this, al::getTrans(this));
    updateCollisionShape();

    sead::Vector3f velocity = al::getVelocity(this);
    f32 tonguePullForce = 0.0f;
    sead::Vector3f tonguePullDirection = {0.0f, 0.0f, 0.0f};
    if (mStateHack->tryCalcTonguePullForce(&tonguePullForce, &tonguePullDirection)) {
        if (rs::isOnGround(this, mCollider)) {
            al::getGravity(this);
            sead::Vector3f groundNormal = rs::getCollidedGroundNormal(mCollider);
            sead::Vector3f pullDirection = groundNormal;
            if (!al::isParallelDirection(groundNormal, tonguePullDirection, 0.01f)) {
                al::verticalizeVec(&pullDirection, groundNormal, tonguePullDirection);
                al::normalize(&pullDirection);
            }

            f32 force;
            f32 forceDirectionRate;
            sead::Vector3f horizontalVelocity;
            sead::Vector3f verticalVelocity;
            forceDirectionRate = pullDirection.dot(tonguePullDirection);
            horizontalVelocity = sead::Vector3f(0.0f, 0.0f, 0.0f);
            force = tonguePullForce;
            verticalVelocity = sead::Vector3f(0.0f, 0.0f, 0.0f);
            {
                sead::Vector3f originalVelocity = velocity;
                al::separateVectorHV(&horizontalVelocity, &verticalVelocity, groundNormal,
                                     originalVelocity);
            }
            f32 horizontalSpeed = horizontalVelocity.length();
            f32 pullSpeed = force * forceDirectionRate;
            f32 currentPullSpeed = pullDirection.dot(horizontalVelocity);

            if (al::isNearZero(currentPullSpeed, 0.001f)) {
                al::verticalizeVec(&velocity, pullDirection, horizontalVelocity);
                f32 targetPullSpeed = sead::Mathf::min(40.0f, pullSpeed);
                velocity.setScaleAdd(targetPullSpeed, pullDirection, velocity);
                f32 velocityLength = velocity.length();
                if (velocityLength > 0.0f)
                    velocity *= horizontalSpeed / velocityLength;
                velocity += verticalVelocity;
            } else if (currentPullSpeed < 0.0f) {
                sead::Vector3f horizontalDirection = {0.0f, 0.0f, 0.0f};
                al::normalize(&horizontalDirection, horizontalVelocity);
                f32 pullRate;
                pullRate = sead::Mathf::abs(horizontalDirection.dot(pullDirection));
                clampGroundPullSpeed(&currentPullSpeed, pullSpeed, horizontalSpeed, pullRate);

                f32 perpendicularSpeed =
                    sqrtf(sead::Mathf::max(0.0f, horizontalSpeed * horizontalSpeed -
                                              currentPullSpeed * currentPullSpeed));
                al::verticalizeVec(&velocity, pullDirection, horizontalVelocity);
                f32 velocityLength = velocity.length();
                if (velocityLength > 0.0f)
                    velocity *= perpendicularSpeed / velocityLength;
                velocity.setScaleAdd(currentPullSpeed, pullDirection, velocity);
                velocity += verticalVelocity;
            } else {
                al::verticalizeVec(&velocity, pullDirection, horizontalVelocity);
                f32 targetPullSpeed = sead::Mathf::min(40.0f, pullSpeed);
                if (currentPullSpeed > targetPullSpeed)
                    targetPullSpeed = currentPullSpeed;
                velocity.setScaleAdd(targetPullSpeed, pullDirection, velocity);
                f32 velocityLength = velocity.length();
                if (velocityLength > 0.0f)
                    velocity *= horizontalSpeed / velocityLength;
                velocity += verticalVelocity;
            }
        } else {
            f32 speed = velocity.length();
            f32 pullSpeed = tonguePullDirection.dot(velocity);
            if (al::isNearZero(pullSpeed, 0.001f)) {
                pullSpeed = sead::Mathf::min(40.0f, tonguePullForce);
            } else {
                sead::Vector3f velocityDirection = {0.0f, 0.0f, 0.0f};
                al::normalize(&velocityDirection, velocity);
                f32 pullRate = sead::Mathf::abs(velocityDirection.dot(tonguePullDirection));
                do {
                    f32 maxPullSpeed;
                    if (pullSpeed < 0.0f) {
                        if (tonguePullForce < pullSpeed)
                            break;
                        maxPullSpeed = sead::Mathf::max(speed, 40.0f) * pullRate;
                        pullSpeed = tonguePullForce;
                        if (!(tonguePullForce > maxPullSpeed))
                            break;
                    } else {
                        pullSpeed = sead::Mathf::max(pullSpeed, tonguePullForce);
                        if (pullSpeed < 0.0f) {
                            pullSpeed = 0.0f;
                            break;
                        }
                        maxPullSpeed = sead::Mathf::max(speed, 40.0f) * pullRate;
                        if (!(pullSpeed > maxPullSpeed))
                            break;
                    }
                    pullSpeed = maxPullSpeed;
                } while (false);
            }

            f32 perpendicularSpeed =
                sqrtf(sead::Mathf::max(0.0f, speed * speed - pullSpeed * pullSpeed));
            al::verticalizeVec(&velocity, tonguePullDirection, velocity);
            f32 velocityLength = velocity.length();
            if (velocityLength > 0.0f)
                velocity *= perpendicularSpeed / velocityLength;
            velocity.setScaleAdd(pullSpeed, tonguePullDirection, velocity);
        }
    }

    if (mTrigger->isOnCollisionExpandCheck())
        rs::resetCollisionExpandCheck(mCollider);

    sead::Vector3f pushedVelocity = {0.0f, 0.0f, 0.0f};
    mPushReceiver->calcPushedVelocity(&pushedVelocity, velocity);
    if (mStateHack->isEnableUpdateCollider())
        rs::updateCollider(this, mCollider, pushedVelocity);

    al::updatePoseTrans(this, al::getTrans(this));
    al::WaterSurfaceFinder* waterSurfaceFinder = mWaterSurfaceFinder;
    const sead::Vector3f& trans = al::getTrans(this);
    const sead::Vector3f& gravity = al::getGravity(this);
    sead::Vector3f up = -gravity;
    waterSurfaceFinder->update(trans, up, 200.0f);
    mEffect->updateWaterSurfaceMtx(mWaterSurfaceFinder);
    sendCollisionMsg();
}

void Yoshi::updateCollisionShape() {
    auto* stateHack = mStateHack;
    PlayerColliderYoshi* collider;
    if (al::isNerve(this, &NrvYoshi.Hack)) {
        bool isTongueJump = stateHack->isCollisionShapeTongueJump();
        collider = mCollider;
        if (isTongueJump) {
            collider->tryChangeCollisionTongueJump();
            return;
        }
    } else {
        collider = mCollider;
    }
    collider->tryChangeCollisionNormal();
}


void Yoshi::sendCollisionMsg() {
    mJumpMessageRequest->clear();
    mTrigger->clearCollisionTrigger();
    if (rs::sendPlayerCollisionUpperPunchMsg(this, mHitSensor, mCollider, mModelChanger))
        mTrigger->set(PlayerTrigger::ECollisionTrigger_val0);
    rs::sendPlayerCollisionTouchMsg(this, mHitSensor, mCollider);

    if (mStateHack->reactionCollidedCollisionCode()) {
        mTrigger->set(PlayerTrigger::ECollisionTrigger_val10);
        al::setNerve(this, &NrvYoshi.Npc);
        return;
    }

    if (mStateNpc->reactionCollidedCollisionCode() ||
        mStateEgg->reactionCollidedCollisionCode()) {
        auto* modelChanger = mModelChanger;
        auto* collider = mCollider;
        auto* localMtx = &mLocalMtx;
        modelChanger->killModel();
        al::updatePoseMtx(this, localMtx);
        al::resetPosition(this);
        rs::resetCollision(collider);
        al::setNerve(this, &NrvYoshi.Egg);
    }
}

void Yoshi::appearEgg() {
    al::setNerve(this, &NrvYoshi.Egg);
    rs::resetCollision(mCollider);
    appear();
    al::invalidateClipping(this);
}

void Yoshi::startFruitShineGetDemo() {
    mStateHack->startFruitShineGetDemo();
}

void Yoshi::exeEgg() {
    if (al::updateNerveState(this))
        al::setNerve(this, &NrvYoshi.Npc);
}

void Yoshi::exeNpc() {
    if (al::updateNerveState(this)) {
        auto* modelChanger = mModelChanger;
        auto* collider = mCollider;
        auto* localMtx = &mLocalMtx;
        modelChanger->killModel();
        al::updatePoseMtx(this, localMtx);
        al::resetPosition(this);
        rs::resetCollision(collider);
        al::setNerve(this, &NrvYoshi.Egg);
    }
}

void Yoshi::exeHack() {
    al::updateNerveState(this);
}


void Yoshi::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &NrvYoshi.Hack) && mStateHack->attackSensor(self, other))
        return;

    if (al::isNerve(this, &NrvYoshi.Npc) && rs::isSensorTypeYoshiEnableSendPush(self) &&
        !al::sendMsgPush(other, self))
        rs::sendMsgPushToPlayer(other, self);
}

// NON_MATCHING: exact 752-byte size; target calls PlayerConst vtable +0x10 while the compatible declaration calls +0x0. Next hypothesis is recovering the specific accessor ABI without shifting already-matching PlayerConst callers.
bool Yoshi::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                       al::HitSensor* self) {
    if (al::isMsgPlayerDisregard(message)) {
        if (al::isNerve(this, &NrvYoshi.Hack))
            return true;
        if (al::isNerve(this, &NrvYoshi.Npc))
            return !al::isSensorName(self, "Head");
    }

    IUsePlayerCollision* collider = mCollider;
    bool canReceivePush = false;
    if (rs::isSensorTypeYoshiMsgReceivable(self)) {
        if (al::isNerve(this, &NrvYoshi.Npc)) {
            if (!rs::isCollidedGround(collider) || rs::isJustLand(collider))
                canReceivePush = true;
        } else if (al::isNerve(this, &NrvYoshi.Hack)) {
            canReceivePush = true;
        }
    }

    if (canReceivePush) {
        bool isHack = al::isNerve(this, &NrvYoshi.Hack);
        f32 gravity = mPlayerConst->getGravity();
        f32 pushPower = isHack ? gravity : gravity * 5.0f;
        if (mPushReceiver->receivePushMsgHacker(message, other, self, pushPower, true))
            return true;
    } else if (al::isNerve(this, &NrvYoshi.Npc) &&
               mPushReceiver->receivePushMsgYoshiNpc(message, other, self,
                                                    mPlayerConst->getGravity() * 5.0f)) {
        return true;
    }

    if (al::isNerve(this, &NrvYoshi.Hack)) {
        bool isReturnEgg = false;
        if (mStateHack->receiveMsgEndState(&isReturnEgg, message, other, self)) {
            if (isReturnEgg) {
                sead::Matrix34f* localMtx = &mLocalMtx;
                PlayerColliderYoshi* colliderYoshi = mCollider;
                mModelChanger->killModel();
                al::updatePoseMtx(this, localMtx);
                al::resetPosition(this);
                rs::resetCollision(colliderYoshi);
                al::setNerve(this, &NrvYoshi.Egg);
            } else {
                mTrigger->set(static_cast<PlayerTrigger::ECollisionTrigger>(10));
                al::setNerve(this, &NrvYoshi.Npc);
            }
            return true;
        }
    }

    if (al::isNerve(this, &NrvYoshi.Npc) && al::isGreaterEqualStep(this, 15) &&
        mStateHack->receiveMsgSetNerveState(message, other, self)) {
        al::setNerve(this, &NrvYoshi.Hack);
        return true;
    }

    if (al::isNerve(this, &NrvYoshi.Npc) &&
        mStateNpc->receiveMsgReturnEggAndInitPosition(message, other, self)) {
        sead::Matrix34f* localMtx = &mLocalMtx;
        PlayerColliderYoshi* colliderYoshi = mCollider;
        mModelChanger->killModel();
        al::updatePoseMtx(this, localMtx);
        al::resetPosition(this);
        rs::resetCollision(colliderYoshi);
        al::setNerve(this, &NrvYoshi.Egg);
        return true;
    }

    if (al::isNerve(this, &NrvYoshi.Hack))
        return mStateHack->receiveMsg(message, other, self);
    if (al::isNerve(this, &NrvYoshi.Npc))
        return mStateNpc->receiveMsg(message, other, self);
    return false;
}
