#include "Player/HackerStateNormalMove.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/HackerJudgeNormalFall.h"
#include "Player/HackerJudgeStartRun.h"
#include "Player/PlayerActionFunction.h"
#include "Player/PlayerActionTurnControl.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackFunction.h"
#include "Util/PlayerHackInputFunction.h"

namespace {
NERVE_IMPL(HackerStateNormalMove, Wait);
NERVE_IMPL(HackerStateNormalMove, Fall);
NERVE_IMPL(HackerStateNormalMove, Move);
NERVE_IMPL(HackerStateNormalMove, Brake);
NERVE_IMPL(HackerStateNormalMove, Pivot);
NERVES_MAKE_STRUCT(HackerStateNormalMove, Wait, Fall, Move, Brake, Pivot);
}  // namespace

HackerStateNormalMove::HackerStateNormalMove(al::LiveActor* actor, IUsePlayerHack** hacker,
                                             const char* waitAnim, const char* runAnim,
                                             const char* fallAnim)
    : al::ActorStateBase("移動", actor), mHacker(hacker), mHackerJudgeNormalFall(nullptr),
      mHackerJudgeStartRun(nullptr), mPlayerActionTurnControl(nullptr), mIUsePlayerCollision(nullptr),
      mIsUsePlayerTurnControl(false), _49(false), _4c(0.7f), _50(1.5f), _54(4.0f), _58(0.99f),
      _5c(0.7f), mHasMoveVelocity(false), mMoveVelocity(0.7f), mHasTurnAngle(false),
      mTurnAngle(20.0f), mMoveDirection(0.0f, 0.0f, 0.0f), mWaitAnim(waitAnim),
      mRunAnim(runAnim), mFallAnim(fallAnim), _98(true), _99(false) {
    initNerve(&NrvHackerStateNormalMove.Wait, 0);
    mHackerJudgeNormalFall = new HackerJudgeNormalFall(actor, 5);
    mHackerJudgeStartRun = new HackerJudgeStartRun(actor, hacker);
    mPlayerActionTurnControl = new PlayerActionTurnControl(actor);
    mPlayerActionTurnControl->setup(3.0f, 45.0f, 10.0f, 10.0f, 20, 1, 30);
}

void HackerStateNormalMove::appear() {
    al::ActorStateBase::appear();
    rs::resetJudge(mHackerJudgeStartRun);

    if (_98) {
        if (rs::updateJudgeAndResult(mHackerJudgeStartRun))
            al::setNerve(this, &NrvHackerStateNormalMove.Move);
        else
            al::setNerve(this, &NrvHackerStateNormalMove.Wait);
    } else {
        al::setNerve(this, &NrvHackerStateNormalMove.Fall);
    }
    _98 = true;
}

void HackerStateNormalMove::setLandNerve() {
    if (rs::updateJudgeAndResult(mHackerJudgeStartRun))
        al::setNerve(this, &NrvHackerStateNormalMove.Move);
    else
        al::setNerve(this, &NrvHackerStateNormalMove.Wait);
}

void HackerStateNormalMove::changeWaitAnim(const char* anim) {
    mWaitAnim = anim;
}

void HackerStateNormalMove::changeRunAnim(const char* anim) {
    mRunAnim = anim;
}

void HackerStateNormalMove::changeFallAnim(const char* anim) {
    mFallAnim = anim;
}

void HackerStateNormalMove::setupFallTime(s32 fallTime) {
    mHackerJudgeNormalFall->set_10(fallTime);
}

void HackerStateNormalMove::usePlayerTurnControl(bool isEnabled) {
    mIsUsePlayerTurnControl = true;
    mPlayerActionTurnControl->set_88(isEnabled);
}

void HackerStateNormalMove::usePlayerCollision(IUsePlayerCollision* playerCollision) {
    mIUsePlayerCollision = playerCollision;
    mHackerJudgeNormalFall->setPlayerCollision(playerCollision);
    mHackerJudgeStartRun->setPlayerCollision(mIUsePlayerCollision);
}

bool HackerStateNormalMove::isWaiting() const {
    return al::isNerve(this, &NrvHackerStateNormalMove.Wait);
}

bool HackerStateNormalMove::isFalling() const {
    return al::isNerve(this, &NrvHackerStateNormalMove.Fall);
}

void HackerStateNormalMove::exeWait() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        if (mWaitAnim)
            al::startAction(actor, mWaitAnim);
        rs::resetJudge(mHackerJudgeNormalFall);
    }

    if (mIUsePlayerCollision)
        rs::tryFollowRotateFrontAxisUpIfCollidedGround(actor, mIUsePlayerCollision);
    else
        al::followRotateFrontAxisUpGround(actor);

    updateGroundWait();
    if (rs::updateJudgeAndResult(mHackerJudgeNormalFall)) {
        al::setNerve(this, &NrvHackerStateNormalMove.Fall);
        return;
    }

    if (tryPivot())
        return;

    if (rs::updateJudgeAndResult(mHackerJudgeStartRun))
        al::setNerve(this, &NrvHackerStateNormalMove.Move);
}

void HackerStateNormalMove::updateGroundWait() {
    al::LiveActor* actor = mActor;
    sead::Vector3f gravityDir;
    if (mIUsePlayerCollision) {
        rs::reboundVelocityFromCollision(actor, mIUsePlayerCollision, 0.0f, 0.0f, 0.0f);
        if (rs::isCollidedGround(mIUsePlayerCollision))
            gravityDir = -rs::getCollidedGroundNormal(mIUsePlayerCollision);
        else
            gravityDir.set(al::getGravity(actor));
    } else {
        al::reboundVelocityFromCollision(actor, 0.0f, 0.0f, 0.0f);
        calcGravityDirLocal(&gravityDir);
    }
    al::addVelocityToDirection(actor, gravityDir, _50);
}

bool HackerStateNormalMove::tryPivot() {
    if (!mHasTurnAngle)
        return false;

    sead::Vector3f moveDir;
    if (!rs::calcHackerMoveDir(&moveDir, *mHacker, sead::Vector3f::ey))
        return false;

    sead::Vector3f front;
    al::calcFrontDir(&front, mActor);
    if (!PlayerActionFunction::isOppositeDir(moveDir, front))
        return false;

    mMoveDirection.set(moveDir);
    al::setNerve(this, &NrvHackerStateNormalMove.Pivot);
    return true;
}

void HackerStateNormalMove::exeMove() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        if (mRunAnim)
            al::startAction(actor, mRunAnim);
        rs::resetJudge(mHackerJudgeNormalFall);
        mPlayerActionTurnControl->reset();
    }

    al::scaleVelocity(actor, _5c);
    sead::Vector3f gravityDir;
    calcGravityDirLocal(&gravityDir);
    al::addVelocityToDirection(actor, gravityDir, _50);

    sead::Vector3f up = sead::Vector3f::ey;
    if (_99) {
        if (mIUsePlayerCollision) {
            rs::calcGroundNormalOrGravityDir(&up, actor, mIUsePlayerCollision);
        } else if (al::isCollidedGround(actor)) {
            up = al::getCollidedGroundNormal(actor);
        } else {
            up = -al::getGravity(actor);
        }
    }

    sead::Vector3f moveVec = sead::Vector3f::zero;
    rs::calcHackerMoveVec(&moveVec, *mHacker, up);
    sead::Vector3f turnDir = {0.0f, 0.0f, 0.0f};
    if (mIsUsePlayerTurnControl) {
        mPlayerActionTurnControl->update(moveVec, up);
        turnDir.set(mPlayerActionTurnControl->get_5c());
        if (mPlayerActionTurnControl->get_68() && !al::tryNormalizeOrZero(&turnDir, moveVec))
            turnDir = mPlayerActionTurnControl->get_50();
    } else if (!al::tryNormalizeOrZero(&turnDir, moveVec)) {
        al::calcFrontDir(&turnDir, actor);
    }

    if (mHasMoveVelocity) {
        sead::Vector3f front;
        al::calcFrontDir(&front, actor);
        if (front.dot(turnDir) < 0.70711f || al::isNearZero(moveVec, 0.001f))
            al::setNerve(this, &NrvHackerStateNormalMove.Brake);
        else
            al::faceToDirection(actor, turnDir);
    } else {
        if (tryPivot())
            return;
        if (_99)
            al::faceToDirectionSupportUp(actor, turnDir);
        else
            al::faceToDirection(actor, turnDir);
    }

    sead::Vector3f velocityH = {0.0f, 0.0f, 0.0f};
    sead::Vector3f velocityV = {0.0f, 0.0f, 0.0f};
    if (_49) {
        al::separateVelocityDirHV(&velocityH, &velocityV, actor, gravityDir);
        al::verticalizeVec(&turnDir, gravityDir, turnDir);
        al::tryNormalizeOrZero(&turnDir);
        al::verticalizeVec(&moveVec, gravityDir, moveVec);
        f32 speed = velocityH.length() + moveVec.length() * _54;
        velocityH = turnDir * speed;
        al::setVelocity(actor, velocityH + velocityV);
    } else {
        al::separateVelocityHV(&velocityH, &velocityV, actor);
        f32 speed = velocityH.length() + moveVec.length() * _54;
        sead::Vector3f scaledVelocity;
        sead::Vector3CalcCommon<f32>::multScalar(scaledVelocity, turnDir, speed);
        al::setVelocity(actor, scaledVelocity + velocityV);
    }
    updateGroundMove();
    if (rs::updateJudgeAndResult(mHackerJudgeNormalFall)) {
        al::setNerve(this, &NrvHackerStateNormalMove.Fall);
        return;
    }

    if (rs::isHackerStopMove(actor, *mHacker, _50))
        al::setNerve(this, &NrvHackerStateNormalMove.Wait);
}

void HackerStateNormalMove::calcGravityDirLocal(sead::Vector3f* gravityDir) const {
    if (!mIUsePlayerCollision) {
        al::calcGravityDir(gravityDir, mActor);
        return;
    }

    rs::calcGroundNormalOrGravityDir(gravityDir, mActor, mIUsePlayerCollision);
    *gravityDir = -*gravityDir;
    al::tryNormalizeOrZero(gravityDir);
}

void HackerStateNormalMove::updateGroundMove() {
    al::LiveActor* actor = mActor;
    const sead::Vector3f* groundNormal;
    if (mIUsePlayerCollision) {
        rs::reboundVelocityPart(actor, mIUsePlayerCollision, 0.0f, 0.0f, 0.0f, 0.0f);
        if (!rs::isCollidedGround(mIUsePlayerCollision))
            return;
        groundNormal = &rs::getCollidedGroundNormal(mIUsePlayerCollision);
    } else {
        al::reboundVelocityPart(actor, 0.0f, 0.0f, 0.0f, 0.0f);
        if (!al::isCollidedGround(actor))
            return;
        groundNormal = &al::getCollidedGroundNormal(actor);
    }
    al::addVelocityToDirection(actor, *groundNormal, -_50);
}

void HackerStateNormalMove::exeBrake() {
    al::LiveActor* actor = mActor;
    al::scaleVelocity(actor, mMoveVelocity);
    sead::Vector3f gravityDir;
    calcGravityDirLocal(&gravityDir);
    al::addVelocityToDirection(actor, gravityDir, _50);

    sead::Vector3f moveVec = sead::Vector3f::zero;
    rs::calcHackerMoveVec(&moveVec, *mHacker, sead::Vector3f::ey);
    bool hasMove = al::tryNormalizeOrZero(&moveVec);
    updateGroundMove();
    if (rs::updateJudgeAndResult(mHackerJudgeNormalFall)) {
        al::setNerve(this, &NrvHackerStateNormalMove.Fall);
        return;
    }

    sead::Vector3f front;
    al::calcFrontDir(&front, actor);
    if (hasMove && front.dot(moveVec) >= 0.76604f) {
        al::setNerve(this, &NrvHackerStateNormalMove.Move);
        return;
    }

    if (tryPivot())
        return;
    if (al::calcSpeedH(actor) < _50) {
        if (!hasMove) {
            al::setNerve(this, &NrvHackerStateNormalMove.Wait);
            return;
        }
        al::faceToDirection(actor, moveVec);
        al::setNerve(this, &NrvHackerStateNormalMove.Move);
    }
}

void HackerStateNormalMove::exePivot() {
    if (al::isFirstStep(this))
        al::setVelocityZeroH(mActor);

    al::scaleVelocity(mActor, _5c);
    sead::Vector3f gravityDir;
    calcGravityDirLocal(&gravityDir);
    al::addVelocityToDirection(mActor, gravityDir, _50);

    sead::Vector3f moveDir;
    if (rs::calcHackerMoveDir(&moveDir, *mHacker, sead::Vector3f::ey))
        mMoveDirection.set(moveDir);

    if (al::turnToDirection(mActor, mMoveDirection, mTurnAngle)) {
        if (al::isNearZero(moveDir, 0.001f))
            al::setNerve(this, &NrvHackerStateNormalMove.Wait);
        else
            al::setNerve(this, &NrvHackerStateNormalMove.Move);
    }
}

void HackerStateNormalMove::exeFall() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this) && mFallAnim)
        al::startAction(actor, mFallAnim);

    al::addVelocityToGravity(actor, _4c);
    al::scaleVelocity(actor, _58);
    if (mIUsePlayerCollision) {
        if (!rs::isOnGround(actor, mIUsePlayerCollision))
            return;
        rs::reboundVelocityFromCollision(actor, mIUsePlayerCollision, 0.0f, 0.0f, 1.0f);
    } else {
        if (!al::isOnGround(actor, 0))
            return;
        al::reboundVelocityFromCollision(actor, 0.0f, 0.0f, 1.0f);
    }
    setLandNerve();
}
