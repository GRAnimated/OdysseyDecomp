#include "Player/PlayerStatePoleClimb.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerActionCollisionSnap.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerJointParamHandLegAngle.h"
#include "Util/JudgeUtil.h"

// NON_MATCHING: behavior is recovered; temporary-vector lifetime and branch layout still differ from the target.
bool PlayerStatePoleClimb::update() {
    rs::updateJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing));

    const f32 repeatAngle = mConst->getPoleClimbInputRepeatAngle();
    if (mInput->isMoveDeepDownNoSnap()) {
        if (!mInput->isSameStickMove(_a8, repeatAngle)) {
            mInput->calcPoleMoveInput(&_9c);
            _a8 = mInput->getStickMoveRaw();
        }
    } else {
        _9c = sead::Vector2f::zero;
        _a8 = sead::Vector2f::zero;
    }

    if (_b4) {
        sead::Vector2f poleMoveInput = sead::Vector2f::zero;
        if (!mInput->isMoveDeepDownNoSnap()) {
            poleMoveInput = sead::Vector2f::zero;
            _b8 = sead::Vector2f::zero;
            _b4 = false;
        } else if (!mInput->isSameStickMove(_b8, repeatAngle)) {
            mInput->calcPoleMoveInput(&poleMoveInput);
            _b8 = mInput->getStickMoveRaw();
            _b4 = false;
        }
    }

    return al::NerveStateBase::update();
}

void PlayerStatePoleClimb::setup(const al::CollisionParts* collisionParts,
                                 const sead::Vector3f& position,
                                 const sead::Vector3f& front,
                                 const sead::Vector3f& up, f32 depth, f32 moveRate,
                                 const char* animationName) {
    updatePoleDepth(depth, moveRate);
    mCollisionSnap->setup(collisionParts, position, front, up);
    mMaterialCode = animationName;
}

// NON_MATCHING: behavior is recovered; getter-call ordering and vector store grouping remain to be refined.
void PlayerStatePoleClimb::updatePoleDepth(f32 depth, f32 moveRate) {
    const f32 minDepth = mConst->getPoleClimbCatchRangeMin();
    const f32 maxDepth = mConst->getPoleClimbCatchRangeMax();
    if (maxDepth < depth)
        depth = maxDepth;
    if (depth < minDepth)
        depth = minDepth;

    _88 = depth;
    _8c = moveRate;

    const f32 centerDepth = mConst->getPoleClimbCatchRange();
    f32 jointAngle = 0.0f;
    if (!al::isNearZero(depth - centerDepth, 0.001f)) {
        if (depth <= centerDepth) {
            const f32 rate = al::calcRate01(depth, mConst->getPoleClimbJointRangeMin(),
                                            centerDepth);
            jointAngle = al::lerpValue(mConst->getPoleClimbJointAngleMin(), 0.0f, rate);
        } else {
            const f32 rate = al::calcRate01(depth, centerDepth,
                                            mConst->getPoleClimbJointRangeMax());
            jointAngle = al::lerpValue(0.0f, mConst->getPoleClimbJointAngleMax(), rate);
        }
    }

    const f32 angle = jointAngle + moveRate;
    mHandLegAngle->handAngle.set(0.0f, angle, 0.0f);
    mHandLegAngle->legAngle.set(0.0f, angle, 0.0f);
}

const sead::Vector3f& PlayerStatePoleClimb::getPoleFront() const {
    return mCollisionSnap->getSnapFront();
}

al::HitSensor* PlayerStatePoleClimb::getPoleSensor() const {
    return mCollisionSnap->tryGetConnectedSensor();
}

bool PlayerStatePoleClimb::followCollision() {
    mCollisionSnap->followCollision();
    return mCollisionSnap->isSnapPartsValid();
}
