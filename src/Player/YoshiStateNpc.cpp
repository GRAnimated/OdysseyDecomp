#include "Player/YoshiStateNpc.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerTrigger.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackFunction.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"
#include "Util/YoshiUtil.h"

namespace {
NERVE_IMPL(YoshiStateNpc, Wait)
NERVE_IMPL(YoshiStateNpc, HackEnd)
NERVE_IMPL(YoshiStateNpc, Appear)
NERVE_IMPL(YoshiStateNpc, Turn)
NERVE_IMPL(YoshiStateNpc, Reaction)

NERVES_MAKE_NOSTRUCT(YoshiStateNpc, Wait)
NERVES_MAKE_STRUCT(YoshiStateNpc, HackEnd, Appear)
NERVES_MAKE_NOSTRUCT(YoshiStateNpc, Turn, Reaction)
}  // namespace

YoshiStateNpc::YoshiStateNpc(al::LiveActor* actor, const PlayerConst* playerConst,
                             const IUsePlayerCollision* collision, const PlayerTrigger* trigger,
                             const al::WaterSurfaceFinder* waterSurfaceFinder,
                             const al::ActorInitInfo& info, PlayerAnimator* animator,
                             al::EventFlowExecutor* eventFlowExecutor)
    : al::ActorStateBase("Npc", actor), mPlayerConst(playerConst), mCollision(collision),
      mTrigger(trigger), mWaterSurfaceFinder(waterSurfaceFinder),
      mEventFlowExecutor(eventFlowExecutor), mAnimator(animator) {
    al::tryGetArg(&mIsUseReturnTimer, info, "IsUseReturnTimer");
    rs::startEventFlow(mEventFlowExecutor, "Init");
    initNerve(&Wait, 0);
}

void YoshiStateNpc::appear() {
    al::ActorStateBase::appear();
    al::invalidateClipping(mActor);
    mIsHackEnd = false;
    mReturnTimer = 0;
    if (mTrigger->isOnYoshiHackEnd()) {
        mIsHackEnd = true;
        al::setNerve(this, &NrvYoshiStateNpc.HackEnd);
    } else {
        al::setNerve(this, &NrvYoshiStateNpc.Appear);
    }
}

void YoshiStateNpc::control() {
    if (rs::isPlayerHack(mActor))
        rs::updateEventFlow(mEventFlowExecutor);

    if (al::isResetablePlayerPos(mActor, 4000.0f)) {
        kill();
        return;
    }

    if (mIsUseReturnTimer && mIsHackEnd) {
        mReturnTimer = al::converge(mReturnTimer, 300, 1);
        if (mReturnTimer == 300) {
            al::startHitReaction(mActor, "[ヨッシー]死亡");
            kill();
        }
    }
}

void YoshiStateNpc::exeAppear() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("Appear");
        if (mAnimator->isUpperBodyAnimAttached())
            mAnimator->clearUpperBodyAnim();
    }

    rs::waitGround(mActor, mCollision, mPlayerConst->getGravity(), mPlayerConst->getFallSpeedMax(),
                   mPlayerConst->getSlerpQuatGrav(), 0.0f);
    if (mAnimator->isAnimEnd())
        al::setNerve(this, &Wait);
}

// NON_MATCHING: target/current 776/768; local stack coloring matches, but target materializes base pointers for the 0x5c and 0x50 vector copies while current emits direct STUR/STR stores; next source-level hypothesis: recover the original sead vector-copy expression/helper that preserves those field-base addresses.
void YoshiStateNpc::exeWait() {
    al::LiveActor* actor = mActor;
    if (rs::isCollidedGroundRunAngle(actor, mCollision, mPlayerConst)) {
        if (!mAnimator->isAnim("Wait"))
            mAnimator->startAnim("Wait");
        rs::waitGround(actor, mCollision, mPlayerConst->getGravity(),
                       mPlayerConst->getFallSpeedMax(), mPlayerConst->getSlerpQuatGrav(), 0.0f);
    } else {
        if (!mAnimator->isAnim("Fall"))
            mAnimator->startAnim("Fall");
        al::tryAddVelocityLimit(actor, al::getGravity(actor) * mPlayerConst->getGravity(),
                                mPlayerConst->getFallSpeedMax());
    }

    mLookAtPlayerOffset.set(0.0f, 0.0f, 0.0f);
    sead::Vector3f front{0.0f, 0.0f, 0.0f};
    sead::Vector3f up{0.0f, 0.0f, 0.0f};
    sead::Vector3f lookDir{0.0f, 0.0f, 0.0f};
    const sead::Vector3f playerOffset = rs::getPlayerHeadPos(actor) - al::getTrans(actor);
    al::calcPoseDir(nullptr, &up, &front, actor);

    al::verticalizeVec(&lookDir, up, playerOffset);
    if (lookDir.length() > 1000.0f || !al::tryNormalizeOrZero(&lookDir))
        return;

    const f32 angle = al::calcAngleOnPlaneDegree(front, lookDir, up);
    const f32 absAngle = sead::Mathf::abs(angle);
    const f32 angleSign = al::sign(angle);
    if (absAngle >= 50.0f) {
        const f32 turnDegree = absAngle > 140.0f ? 180.0f : 90.0f;
        mTurnStartFront.set(front);
        al::rotateVectorDegree(&mTurnTargetFront, front, up, angleSign * turnDegree);
        mTurnUp.set(up);
        al::setNerve(this, &Turn);
    } else {
        mLookAtPlayerOffset.setScaleAdd(0.0f, up, playerOffset);
    }
}

void YoshiStateNpc::exeTurn() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this))
        mAnimator->startAnim("TurnPoint");

    const sead::Vector3f oldUp = mTurnUp;
    al::calcUpDir(&mTurnUp, actor);
    al::alongVectorNormalH(&mTurnStartFront, mTurnStartFront, oldUp, mTurnUp);
    al::alongVectorNormalH(&mTurnTargetFront, mTurnTargetFront, oldUp, mTurnUp);

    sead::Vector3f front{0.0f, 0.0f, 0.0f};
    al::turnVecToVecRate(&front, mTurnStartFront, mTurnTargetFront,
                         al::calcNerveEaseOutRate(this, 15));
    sead::Quatf quat = sead::Quatf::unit;
    al::makeQuatFrontUp(&quat, front, mTurnUp);
    al::updatePoseQuat(actor, quat);

    rs::waitGround(actor, mCollision, mPlayerConst->getGravity(), mPlayerConst->getFallSpeedMax(),
                   mPlayerConst->getSlerpQuatGrav(), 0.0f);
    if (al::isGreaterEqualStep(this, 15))
        al::setNerve(this, &Wait);
}

void YoshiStateNpc::exeReaction() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this))
        mAnimator->startAnim("Reaction");

    rs::waitGround(actor, mCollision, mPlayerConst->getGravity(), mPlayerConst->getFallSpeedMax(),
                   mPlayerConst->getSlerpQuatGrav(), 0.0f);
    if (mAnimator->isAnimEnd())
        al::setNerve(this, &Wait);
}

void YoshiStateNpc::exeHackEnd() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("HackEnd");
        if (mAnimator->isUpperBodyAnimAttached())
            mAnimator->clearUpperBodyAnim();
    }

    al::LiveActor* actor = mActor;
    if (rs::isCollidedGround(mCollision)) {
        rs::waitGround(actor, mCollision, mPlayerConst->getGravity(),
                       mPlayerConst->getFallSpeedMax(), mPlayerConst->getSlerpQuatGrav(), 0.0f);
    } else {
        al::tryAddVelocityLimit(actor, al::getGravity(actor) * mPlayerConst->getGravity(),
                                mPlayerConst->getFallSpeedMax());
    }

    if (mAnimator->isAnimEnd())
        al::setNerve(this, &Wait);
}

bool YoshiStateNpc::tryGetLookAtPlayerPos(sead::Vector3f* position) const {
    if (!al::isNerve(this, &Wait) || al::isNearZero(mLookAtPlayerOffset))
        return false;

    const sead::Vector3f& trans = al::getTrans(mActor);
    position->setAdd(trans, mLookAtPlayerOffset);
    return true;
}

bool YoshiStateNpc::reactionCollidedCollisionCode() {
    if (isDead())
        return false;

    al::LiveActor* actor = mActor;
    if (al::isInDeathArea(actor) || rs::isTouchHackCancelCollisionCode(actor, mCollision)) {
        al::startHitReaction(actor, "[ヨッシー]死亡");
        return true;
    }

    if (al::isInWater(actor) && !rs::isInPuddleHeight(mWaterSurfaceFinder, mPlayerConst)) {
        al::startHitReaction(actor, "[ヨッシー]死亡");
        return true;
    }

    return false;
}

bool YoshiStateNpc::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                               al::HitSensor* self) {
    if ((!al::isMsgExplosion(message) && !al::isMsgBlockUpperPunch(message) &&
         !rs::checkMsgNpcTrampleReactionAll(message, other, self, false) &&
         !rs::isMsgHackNpcCapReactionAll(message)) ||
        (al::isNerve(this, &Reaction) && !al::isGreaterEqualStep(this, 6)))
        return false;

    rs::requestHitReactionToAttacker(message, self, other);
    al::setNerve(this, &Reaction);
    return true;
}

bool YoshiStateNpc::receiveMsgReturnEggAndInitPosition(const al::SensorMsg* message,
                                                       al::HitSensor* other, al::HitSensor* self) {
    if (rs::isMsgFireSwitchFire(message) || rs::isMsgGunetterAttack(message) ||
        rs::isMsgTankHackTrample(message) || rs::isMsgTankKickEnemy(message)) {
        al::startHitReaction(mActor, "[ヨッシー]死亡");
        return true;
    }

    if (al::isMsgGoalKill(message))
        return true;
    return rs::isMsgKillByHomeDemo(message);
}

YoshiStateNpc::~YoshiStateNpc() = default;
