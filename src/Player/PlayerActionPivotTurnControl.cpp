#include "Player/PlayerActionPivotTurnControl.h"

#include <math/seadQuat.h>

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/IUsePlayerHack.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"

PlayerActionPivotTurnControl::PlayerActionPivotTurnControl(
    al::LiveActor* player, const PlayerConst* playerConst, const PlayerInput* input,
    const IUsePlayerCollision* collision, f32 gravity)
    : mPlayer(player), mConst(playerConst), mInput(input), mCollision(collision),
      mPlayerHack(nullptr), _28(sead::Vector3f::zero), _34(gravity),
      _38(sead::Vector3f::zero), _44(false), _45(false), _48(0) {}

void PlayerActionPivotTurnControl::reset() {
    _44 = false;
    _45 = false;
    _48 = 0;
    al::calcFrontDir(&_38, mPlayer);
    rs::calcGroundNormalOrGravityDir(&_28, mPlayer, mCollision);
}

// NON_MATCHING: exact 0x428 target size; first mismatch at 0x7100418D54 in the gravity-subtraction load/register order; next hypothesis is a validator-safe vector source shape that emits the target LDP/STR/STP schedule.
void PlayerActionPivotTurnControl::update() {
    sead::Vector3f previousUp = _28;
    rs::calcGroundNormalOrGravityDir(&_28, mPlayer, mCollision);

    sead::Vector3f moveDirection = {0.0f, 0.0f, 0.0f};
    calcMoveDirection(&moveDirection, _28);

    sead::Vector3f velocity = al::getVelocity(mPlayer);
    if (rs::isOnGround(mPlayer, mCollision))
        al::alongVectorNormalH(&velocity, al::getVelocity(mPlayer), previousUp, _28);

    sead::Vector3f gravityVelocity = _28;
    gravityVelocity *= _34;
    velocity -= gravityVelocity;
    al::setVelocity(mPlayer, velocity);

    {
        al::LiveActor* player = mPlayer;
        const sead::Vector3f gravityDir = -_28;
        al::limitVelocityDir(player, gravityDir, mConst->getFallSpeedMax());
    }

    const bool isMoveDirectionZero = al::isNearZero(moveDirection, 0.001f);
    _44 = !isMoveDirectionZero;
    if (isMoveDirectionZero)
        al::alongVectorNormalH(&_38, _38, previousUp, _28);
    else
        _38 = moveDirection;

    if (!al::tryNormalizeOrZero(&_38)) {
        _45 = true;
        return;
    }

    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, mPlayer);
    al::verticalizeVec(&front, _28, front);
    if (!al::tryNormalizeOrZero(&front)) {
        _45 = true;
        return;
    }

    if (_48 + 1 <= 10)
        _48++;
    else
        _48 = 10;

    sead::Quatf quat = sead::Quatf::unit;
    al::makeQuatAxisRotation(&quat, front, _38, _28, static_cast<f32>(_48) / 10.0f);
    front.rotate(quat);
    al::tryNormalizeOrZero(&front);

    if (al::makeQuatRotationLimit(&quat, front, _38, sead::Mathf::deg2rad(5.0f))) {
        rs::slerpUpFront(mPlayer, _28, front, mConst->getSlerpQuatRate(),
                         mConst->getHillPoseDegreeMax());
        return;
    }

    sead::Vector3f velocityUp = {0.0f, 0.0f, 0.0f};
    sead::Vector3f velocityHorizontal = {0.0f, 0.0f, 0.0f};
    al::separateVectorParallelVertical(&velocityUp, &velocityHorizontal, _28,
                                       al::getVelocity(mPlayer));
    const f32 horizontalSpeed = velocityHorizontal.length();
    al::setVelocity(mPlayer, velocityUp + horizontalSpeed * _38);
    rs::slerpUpFront(mPlayer, _28, _38, mConst->getSlerpQuatRate(),
                     mConst->getHillPoseDegreeMax());
    _45 = true;
}

void PlayerActionPivotTurnControl::calcMoveDirection(sead::Vector3f* moveDirection,
                                                      const sead::Vector3f& up) {
    if (mPlayerHack)
        rs::calcHackerMoveDir(moveDirection, *mPlayerHack, up);
    else
        mInput->calcMoveDirection(moveDirection, up);
}
