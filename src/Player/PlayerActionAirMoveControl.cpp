#include "Player/PlayerActionAirMoveControl.h"

#include <algorithm>
#include <cstring>

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/IUsePlayerHack.h"
#include "Player/PlayerActionTurnControl.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerHackInputFunction.h"

PlayerActionAirMoveControl::PlayerActionAirMoveControl(al::LiveActor* actor,
                                                       const PlayerConst* playerConst,
                                                       const PlayerInput* input,
                                                       const IUsePlayerCollision* collision,
                                                       bool isSlerpGravity)
    : mActor(actor), mConst(playerConst), mInput(input), mCollision(collision), _20(nullptr),
      mTurnControl(nullptr), _30(isSlerpGravity), mIsHoldJumpExtend(false), _32(false), _33(false),
      mExtendFrame(0), _38(0), _3c(0), _40(false), _41(false), _42(false), _44(0.0f), _48(0.0f),
      mStartMoveDir(sead::Vector3f::zero), _58(sead::Vector3f::zero), _64(0.0f), _68(0.0f),
      _6c(0.0f), _70(false), _74(1.0f), _78(0.0f), _7c(0.0f), _80(0.0f), _84{0.0f, 0.0f, 0.0f} {
    mTurnControl = new PlayerActionTurnControl(actor);
    mTurnControl->set_88(true);
    mTurnControl->set_89(true);
    mTurnControl->setup(mConst->getJumpTurnAngleStart(), mConst->getJumpTurnAngleFast(),
                        mConst->getJumpTurnAngleLimit(), mConst->getJumpTurnAngleFastLimit(),
                        mConst->getJumpTurnAccelFrame(), mConst->getJumpTurnAccelFrameFast(),
                        mConst->getJumpTurnBrakeFrame());
}

// NON_MATCHING: behavior complete; current is 0x340 versus target 0x348 (difflib score 670).
// Direct member cross construction restores the target 0xa0 stack frame, and delaying the hold-jump
// member store restores the target clamp-result register. The remaining size gap is one paired cross
// store replacing two scalar stores and LLVM reusing the first frame comparison instead of emitting
// the target CMP W20, #1. Next hypothesis is a natural vector/clamp lifetime that prevents both folds.
void PlayerActionAirMoveControl::setup(f32 speedMax, f32 inertiaAdd, s32 extendFrame, f32 velocityV,
                                       f32 gravityAccel, s32 noInputFrame, f32 inertiaRate) {
    _68 = gravityAccel;
    _3c = noInputFrame;
    _6c = mConst->getFallSpeedMax();

    sead::Vector3f velocityH = {0.0f, 0.0f, 0.0f};
    sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
    if (_40)
        rs::calcGroundNormalOrUpDir(&groundNormal, mActor, mCollision);
    else
        rs::calcGroundNormalOrGravityDir(&groundNormal, mActor, mCollision);

    al::verticalizeVec(&velocityH, groundNormal, al::getVelocity(mActor));
    al::verticalizeVec(&velocityH, al::getGravity(mActor), velocityH);
    if (!al::tryNormalizeOrZero(&mStartMoveDir, velocityH)) {
        al::calcFrontDir(&mStartMoveDir, mActor);
        al::verticalizeVec(&mStartMoveDir, al::getGravity(mActor), mStartMoveDir);
        if (!al::tryNormalizeOrZero(&mStartMoveDir))
            al::calcUpDir(&mStartMoveDir, mActor);
    }

    _58.setCross(mStartMoveDir, al::getGravity(mActor));
    al::normalize(&_58);

    f32 speed = velocityH.length();
    if (_42) {
        if (speed < _44)
            speed = _44;
        else if (speed > _48)
            speed = _48;
    }

    sead::Vector3f inertia;
    inertia.set(0.0f, 0.0f, 0.0f);
    sead::Vector3f moveVelocity;
    moveVelocity.set(0.0f, 0.0f, 0.0f);
    if (speed >= mConst->getNormalMinSpeed())
        sead::Vector3CalcCommon<f32>::multScalar(moveVelocity, mStartMoveDir, speed);

    if (_41)
        rs::calcJumpInertiaWall(&inertia, mActor, mCollision, inertiaRate);
    else
        rs::calcJumpInertia(&inertia, mActor, mCollision, moveVelocity, inertiaRate);

    f32 moveSpeed = inertia.length() + inertiaAdd;
    if (moveSpeed > speedMax)
        moveSpeed = speedMax;
    const f32 startSpeed = speed <= speedMax ? speed : speedMax;
    if (startSpeed >= moveSpeed)
        moveSpeed = startSpeed;
    _64 = moveSpeed;

    al::LiveActor* actor = mActor;
    const sead::Vector3f velocity =
        startSpeed * mStartMoveDir - velocityV * al::getGravity(actor) + inertia;
    al::setVelocity(actor, velocity);

    const bool isHoldJumpExtend = extendFrame > 0;
    extendFrame = std::max(extendFrame, 0);
    mIsHoldJumpExtend = isHoldJumpExtend;
    mExtendFrame = extendFrame;
    _32 = false;
    _38 = 0;
    _80 = mConst->getSlerpQuatRate();
    _84.set(0.0f, 0.0f, 0.0f);
    mTurnControl->reset();
}

void PlayerActionAirMoveControl::setupTurn(f32 angleStart, f32 angleFast, f32 angleLimit,
                                           f32 angleFastLimit, s32 accelFrame, s32 accelFrameFast,
                                           s32 brakeFrame) {
    mTurnControl->setup(angleStart, angleFast, angleLimit, angleFastLimit, accelFrame,
                        accelFrameFast, brakeFrame);
    mTurnControl->reset();
}

void PlayerActionAirMoveControl::setExtendFrame(s32 frame) {
    mExtendFrame = frame;
}

void PlayerActionAirMoveControl::setupCollideWallScaleVelocity(f32 scaleFront, f32 scaleSide,
                                                               f32 scaleUp) {
    _70 = true;
    _74 = scaleFront;
    _78 = scaleSide;
    _7c = scaleUp;
}

void PlayerActionAirMoveControl::verticalizeStartMoveDir(const sead::Vector3f& vertical) {
    sead::Vector3f moveDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f* startMoveDir = &mStartMoveDir;
    const sead::Vector3f* verticalDir = &vertical;
    al::verticalizeVec(&moveDir, *verticalDir, *startMoveDir);
    if (!al::tryNormalizeOrZero(&moveDir)) {
        std::memcpy(&moveDir, &_58, sizeof(moveDir));
        al::verticalizeVec(&moveDir, *verticalDir, _58);
        if (!al::tryNormalizeOrZero(&moveDir))
            return;
    }

    const sead::Vector3f up = -al::getGravity(mActor);
    if (al::isParallelDirection(up, moveDir, 0.01f))
        return;

    std::memcpy(startMoveDir, &moveDir, sizeof(moveDir));
    _58.setCross(up, *startMoveDir);
    al::normalize(&_58);
}

void PlayerActionAirMoveControl::update() {
    if (_70)
        rs::scaleVelocityInertiaWallHit(mActor, mCollision, _74, _78, _7c);

    const s32 nextFrame = _38 + 1;
    const s32 maxFrame = mExtendFrame + 1;
    _38 = nextFrame > maxFrame ? maxFrame : nextFrame;
    if (mIsHoldJumpExtend && _38 <= mExtendFrame)
        mIsHoldJumpExtend = isHoldJumpExtend();
    else
        al::addVelocityToGravity(mActor, _68);

    const sead::Vector3f gravity = al::getGravity(mActor);
    sead::Vector3f moveInput = {0.0f, 0.0f, 0.0f};
    if (_3c == 0) {
        {
            const sead::Vector3f up = -gravity;
            calcMoveInput(&moveInput, up);
        }

        al::verticalizeVec(&mStartMoveDir, gravity, mStartMoveDir);
        if (!al::tryNormalizeOrZero(&mStartMoveDir)) {
            al::calcFrontDir(&mStartMoveDir, mActor);
            al::verticalizeVec(&mStartMoveDir, gravity, mStartMoveDir);
            if (!al::tryNormalizeOrZero(&mStartMoveDir))
                al::calcUpDir(&mStartMoveDir, mActor);

            _58 = (-gravity).cross(mStartMoveDir);
            al::normalize(&_58);
        }

        f32 velocityFront = al::getVelocity(mActor).dot(mStartMoveDir);
        if (velocityFront < 0.0f)
            mStartMoveDir = -mStartMoveDir;

        const f32 inputFront = mStartMoveDir.dot(moveInput);
        const f32 accel = inputFront > 0.0f || al::isNearZero(velocityFront, 0.001f) ?
                              mConst->getJumpAccelFront() :
                              mConst->getJumpAccelBack();
        al::addVelocity(mActor, accel * (inputFront * mStartMoveDir));
        const sead::Vector3f moveInputCopy = moveInput;
        const sead::Vector3f inputSide = moveInputCopy - inputFront * mStartMoveDir;
        al::addVelocity(mActor, mConst->getJumpAccelTurn() * inputSide);
    }

    al::limitVelocityDir(mActor, mStartMoveDir, _64);
    al::limitVelocityDir(mActor, _58, _64);
    if (al::getVelocity(mActor).dot(gravity) > 0.0f)
        al::limitVelocityDir(mActor, gravity, _6c);

    if (_30) {
        rs::slerpGravity(mActor, mConst->getSlerpQuatRate());
    } else {
        const bool isMoveInputZero = al::isNearZero(moveInput, 0.001f);
        PlayerActionTurnControl* turnControl = mTurnControl;
        if (isMoveInputZero) {
            const sead::Vector3f* turnInput = &_84;
            const sead::Vector3f up = -gravity;
            turnControl->update(*turnInput, up);
        } else {
            const sead::Vector3f* turnInput = &moveInput;
            const sead::Vector3f up = -gravity;
            turnControl->update(*turnInput, up);
        }

        if (_33) {
            al::LiveActor* actor = mActor;
            const sead::Vector3f up = -gravity;
            PlayerActionTurnControl* poseTurnControl = mTurnControl;
            rs::slerpSkyPoseAirSnapSide(actor, up, poseTurnControl->get_5c(), mConst->getTall(), _80);
        } else {
            PlayerActionTurnControl* poseTurnControl = mTurnControl;
            al::LiveActor* actor = mActor;
            if (poseTurnControl->get_6a()) {
                const sead::Vector3f up = -gravity;
                rs::slerpSkyPoseAir(actor, up, poseTurnControl->get_5c(), _80);
            } else {
                const sead::Vector3f up = -gravity;
                rs::slerpUpFront(actor, up, poseTurnControl->get_5c(), _80, 0.0f);
            }
        }
    }

    s32 noInputFrame = --_3c;
    if (noInputFrame < 0)
        noInputFrame = 0;
    _3c = noInputFrame;
}

bool PlayerActionAirMoveControl::isHoldJumpExtend() const {
    if (_32)
        return true;
    if (_20)
        return rs::isHoldHackJump(*_20);
    return mInput->isHoldJump();
}

void PlayerActionAirMoveControl::calcMoveInput(sead::Vector3f* moveInput,
                                               const sead::Vector3f& up) const {
    if (_20)
        rs::calcHackerMoveVec(moveInput, *_20, up);
    else if (_33)
        mInput->calc2DSnapJumpMoveInput(moveInput, up);
    else
        mInput->calcMoveInput(moveInput, up);
}
