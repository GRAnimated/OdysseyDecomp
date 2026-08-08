#include "Player/PlayerInput.h"

#include <prim/seadMemUtil.h>

#include "Library/Controller/InputFunction.h"
#include "Library/Controller/JoyPadAccelPoseAnalyzer.h"
#include "Library/Controller/SpinInputAnalyzer.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerFunction.h"
#include "Player/PlayerInputFunction.h"
#include "Util/ActorDimensionUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/StageInputFunction.h"

namespace {
// NON_MATCHING: target orders the repeated up-direction temporary differently; next source-level
// hypothesis is the original helper condition and vector lifetime order.
bool isInvalid2DSnapJumpMoveInput(const sead::Vector3f& input, const al::LiveActor* actor,
                                  const IUsePlayerCollision* collision) {
    sead::Vector3f up = sead::Vector3f::zero;
    al::calcUpDir(&up, actor);
    al::normalize(&up);
    if (input.dot(up) < -0.86603f)
        return true;

    up = sead::Vector3f::zero;
    al::calcUpDir(&up, actor);
    al::normalize(&up);
    f32 dot = input.dot(up);
    return dot >= (rs::isCollidedGround(collision) ? 0.98481f : 0.86603f);
}
}  // namespace

void PlayerInput::updateSnapMoveArea() {
    _38 = mPlayerCollision && rs::tryFindSnapMoveAreaDir(&_3c, mLiveActor, mPlayerCollision);
}

sead::Vector2f PlayerInput::getMoveInputRaw(bool isSeparateCap) const {
    if (_98)
        return sead::Vector2f::zero;

    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    s32 stick = 0;
    if (rs::isSeparatePlay(mLiveActor)) {
        if (isSeparateCap) {
            port = al::getPlayerControllerPort(1);
            stick = _90;
        } else {
            port = al::getPlayerControllerPort(0);
            stick = _8c;
        }
    }
    return PlayerInputFunction::getMoveInputStick(mLiveActor, port, stick);
}

void PlayerInput::resetAlongWall() {
    _1c = 0;
    _20 = 0.0f;
}

// NON_MATCHING: behavior is recovered, but branch and temporary scheduling differ; next source-level
// hypothesis is the original early-return condition grouping.
bool PlayerInput::isNoInput() const {
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    if (al::isPadHoldAny(port))
        return false;
    if (al::getLeftStick(port).length() >= 0.1f)
        return false;
    return al::getRightStick(port).length() < 0.1f &&
           !mJoyPadAccelPoseAnalyzer1->isSwingAnyHand();
}

// NON_MATCHING: behavior is recovered, but controller and stick temporaries differ; next source-level
// hypothesis is the original separate-play branch order.
bool PlayerInput::isNoInputSeparateCap() const {
    if (!rs::isSeparatePlay(mLiveActor))
        return true;
    if (isHoldCapSeparateJump() || isHoldCapSeparateHipDrop())
        return false;

    s32 port = al::getPlayerControllerPort(1);
    if (PlayerInputFunction::isHoldAction(mLiveActor, port))
        return false;

    sead::Vector2f input = sead::Vector2f::zero;
    if (!_98) {
        s32 stick = 0;
        s32 inputPort = PlayerFunction::getPlayerInputPort(mLiveActor);
        if (rs::isSeparatePlay(mLiveActor)) {
            inputPort = al::getPlayerControllerPort(1);
            stick = _90;
        }
        input = PlayerInputFunction::getMoveInputStick(mLiveActor, inputPort, stick);
    }
    return al::isNearZero(input, 0.001f);
}

bool PlayerInput::isHoldCapSeparateJump() const {
    if (_98 || !rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = al::getPlayerControllerPort(1);
    return PlayerInputFunction::isHoldJump(mLiveActor, port);
}

bool PlayerInput::isHoldCapSeparateHipDrop() const {
    if (_98 || !rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = al::getPlayerControllerPort(1);
    return PlayerInputFunction::isHoldSubAction(mLiveActor, port);
}

s32 PlayerInput::getSeparatePlay2P() {
    return al::getPlayerControllerPort(1);
}

bool PlayerInput::isMove() const {
    return !_98 && _18;
}

bool PlayerInput::isMoveDeepDown() const {
    if (_98 || !_18)
        return false;
    sead::Vector2f input = getStickMoveRaw();
    return input.squaredLength() > 0.64f;
}

sead::Vector2f PlayerInput::getStickMoveRaw() const {
    if (_98)
        return sead::Vector2f::zero;

    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    s32 stick = 0;
    if (rs::isSeparatePlay(mLiveActor)) {
        port = al::getPlayerControllerPort(0);
        stick = _8c;
    }
    return PlayerInputFunction::getMoveInputStick(mLiveActor, port, stick);
}

bool PlayerInput::isMoveDeepDownNoSnap() const {
    if (_98)
        return false;
    sead::Vector2f input = getStickMoveRaw();
    return input.squaredLength() > 0.64f;
}

bool PlayerInput::isEnableCarry() const {
    return isHoldAction();
}

bool PlayerInput::isTriggerCarryStart() const {
    return isTriggerAction();
}

bool PlayerInput::isTriggerCarryRelease() const {
    return isTriggerAction() || isTriggerSwingActionMario();
}

bool PlayerInput::isTriggerSwingActionMario() const {
    if (_98)
        return false;

    return mJoyPadAccelPoseAnalyzer1->isSwingAnyHand();
}

bool PlayerInput::isTriggerCarryReleaseBySwing() const {
    if (_98)
        return false;
    if (!PlayerInputFunction::isTriggerAction(mLiveActor,
                                              PlayerFunction::getPlayerInputPort(mLiveActor))) {
        if (_98)
            return false;
        if (!mJoyPadAccelPoseAnalyzer1->isSwingAnyHand())
            return false;
    }
    if (_98)
        return false;
    return mJoyPadAccelPoseAnalyzer1->isSwingAnyHand();
}

bool PlayerInput::isTriggerAction() const {
    if (_98)
        return false;
    return PlayerInputFunction::isTriggerAction(mLiveActor,
                                                PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isTriggerJump() const {
    if (_98)
        return false;
    auto* dimension = mDimension;
    if (dimension && rs::is2D(dimension) && rs::isIn2DArea(dimension)) {
        if (!_98 && mJoyPadAccelPoseAnalyzer1->isSwingAnyHand())
            return true;
    }
    return PlayerInputFunction::isTriggerJump(mLiveActor,
                                              PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isTriggerHipDrop() const {
    if (_98)
        return false;
    return PlayerInputFunction::isTriggerSubAction(mLiveActor,
                                                   PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isTriggerHeadSliding() const {
    return isTriggerCarryRelease();
}

bool PlayerInput::isTriggerPaddle() const {
    if (_98)
        return false;
    return PlayerInputFunction::isTriggerJump(mLiveActor,
                                              PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isTriggerRolling(bool isForceRolling) const {
    if (_98)
        return false;
    if (!PlayerInputFunction::isHoldSubAction(mLiveActor,
                                              PlayerFunction::getPlayerInputPort(mLiveActor)) &&
        !isForceRolling)
        return false;
    if (PlayerInputFunction::isTriggerAction(mLiveActor,
                                             PlayerFunction::getPlayerInputPort(mLiveActor)))
        return true;
    if (_98)
        return false;
    return mJoyPadAccelPoseAnalyzer1->isSwingAnyHand();
}

bool PlayerInput::isTriggerRollingRestartSwing() const {
    return isTriggerSwingActionMario();
}

bool PlayerInput::isTriggerRollingCancelHipDrop(bool isForceRolling) const {
    return isTriggerRolling(isForceRolling);
}

bool PlayerInput::isTriggerHackAction() const {
    return isTriggerAction();
}

bool PlayerInput::isTriggerHackJump() const {
    return isTriggerPaddle();
}

bool PlayerInput::isTriggerHackSwing() const {
    return isTriggerSwingActionMario();
}

bool PlayerInput::isTriggerHackEnd() const {
    if (_98)
        return false;
    u32 inputPort = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isTriggerSubAction(mLiveActor, inputPort);
}

bool PlayerInput::isTriggerHackSeparateJump() const {
    if (_98)
        return false;
    if (!rs::isSeparatePlay(mLiveActor))
        return false;
    s32 inputPort = al::getPlayerControllerPort(1);
    return PlayerInputFunction::isTriggerJump(mLiveActor, inputPort);
}

bool PlayerInput::isTriggerSeparateCapJangoHelp() const {
    if (_98)
        return false;
    if (!rs::isSeparatePlay(mLiveActor))
        return false;
    s32 inputPort = al::getPlayerControllerPort(1);
    return PlayerInputFunction::isTriggerAction(mLiveActor, inputPort) ||
           PlayerInputFunction::isTriggerJump(mLiveActor, inputPort) ||
           PlayerInputFunction::isTriggerSubAction(mLiveActor, inputPort);
}

bool PlayerInput::isHoldHackSeparateJump() const {
    if (_98)
        return false;
    if (!rs::isSeparatePlay(mLiveActor))
        return false;
    s32 inputPort = al::getPlayerControllerPort(1);
    return PlayerInputFunction::isHoldJump(mLiveActor, inputPort);
}

bool PlayerInput::isTriggerGetOff() const {
    return isTriggerHipDrop();
}

bool PlayerInput::isHoldAction() const {
    if (_98)
        return false;
    return PlayerInputFunction::isHoldAction(mLiveActor,
                                             PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isHoldJump() const {
    if (_98)
        return false;
    auto* dimension = mDimension;
    if (dimension && rs::is2D(dimension) && rs::isIn2DArea(dimension) && _88 > 0)
        return true;
    return PlayerInputFunction::isHoldJump(mLiveActor,
                                           PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isHoldHipDrop() const {
    if (_98)
        return false;
    return PlayerInputFunction::isHoldSubAction(mLiveActor,
                                                PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isTriggerStartTalk() const {
    if (_98)
        return false;
    return PlayerInputFunction::isTriggerTalk(mLiveActor,
                                              PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isTriggerStartWorldWarp() const {
    if (_98)
        return false;
    return PlayerInputFunction::isTriggerStartWorldWarp(
        mLiveActor, PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isTriggerCancelWorldWarp() const {
    if (_98)
        return false;
    return PlayerInputFunction::isTriggerCancelWorldWarp(
        mLiveActor, PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isTriggerSpinCap() const {
    if (_98)
        return false;
    u32 inputPort = PlayerFunction::getPlayerInputPort(mLiveActor);
    if (PlayerInputFunction::isTriggerAction(mLiveActor, inputPort))
        return true;
    if (_98)
        return false;
    return mJoyPadAccelPoseAnalyzer1->isSwingAnyHand();
}

bool PlayerInput::isTriggerToggleStayCap() const {
    if (_98)
        return false;
    if (!rs::isSeparatePlay(mLiveActor))
        return false;
    s32 inputPort = al::getPlayerControllerPort(1);
    return PlayerInputFunction::isTriggerAction(mLiveActor, inputPort);
}

bool PlayerInput::isTriggerSpinAttackSeparate() const {
    if (_98)
        return false;
    if (!rs::isSeparatePlay(mLiveActor))
        return false;
    if (!_98 && mJoyPadAccelPoseAnalyzer1->isSwingAnyHand())
        return true;
    return PlayerInputFunction::isTriggerAction(mLiveActor, al::getPlayerControllerPort(0));
}

s32 PlayerInput::getSeparatePlay1P() {
    return al::getPlayerControllerPort(0);
}

bool PlayerInput::isTriggerCapReturn() const {
    if (_98)
        return false;
    if (!rs::isSeparatePlay(mLiveActor))
        return isTriggerSpinCap();
    return isTriggerCapAttackSeparate();
}

bool PlayerInput::isTriggerCapAttackSeparate() const {
    if (_98 || !rs::isSeparatePlay(mLiveActor))
        return false;
    s32 inputPort = al::getPlayerControllerPort(1);
    if (PlayerInputFunction::isTriggerAction(mLiveActor, inputPort))
        return true;
    if (_98 || !rs::isSeparatePlay(mLiveActor))
        return false;
    return mJoyPadAccelPoseAnalyzer2->isSwingAnyHand();
}

bool PlayerInput::isTriggerSwingActionCap() const {
    if (_98 || !rs::isSeparatePlay(mLiveActor))
        return false;
    return mJoyPadAccelPoseAnalyzer2->isSwingAnyHand();
}

bool PlayerInput::isTriggerCapSingleHandThrow() const {
    return isTriggerSwingActionMario();
}

bool PlayerInput::isTriggerCapDoubleHandThrow() const {
    if (_98)
        return false;
    return mJoyPadAccelPoseAnalyzer1->isSwingDoubleHandSameDir();
}

bool PlayerInput::isTriggerCapSeparateJump() const {
    return isTriggerHackSeparateJump();
}

bool PlayerInput::isTriggerCapSeparateHipDrop() const {
    if (_98)
        return false;
    if (!rs::isSeparatePlay(mLiveActor))
        return false;
    s32 inputPort = al::getPlayerControllerPort(1);
    return PlayerInputFunction::isTriggerSubAction(mLiveActor, inputPort);
}

bool PlayerInput::isTriggerSwingPoleClimbFast() const {
    return isTriggerSwingActionMario();
}

bool PlayerInput::isHoldPoleClimbDown() const {
    return isHoldHipDrop();
}

bool PlayerInput::isTriggerAppendCapAttack(bool isSeparatePlay) const {
    if (_98)
        return false;
    if (!rs::isSeparatePlay(mLiveActor) || isSeparatePlay) {
        if (_98)
            return false;
        return mJoyPadAccelPoseAnalyzer1->isSwingAnyHand();
    }
    if (_98 || !rs::isSeparatePlay(mLiveActor))
        return false;
    return mJoyPadAccelPoseAnalyzer2->isSwingAnyHand();
}

// NON_MATCHING: behavior is recovered, but dimension and vector branches differ; next source-level
// hypothesis is the original nested-condition order.
bool PlayerInput::isHoldSquat() const {
    if (_98)
        return false;

    const IUseDimension* dimension = mDimension;
    if (dimension && rs::is2D(dimension) && rs::isIn2DArea(dimension)) {
        if (!_cd) {
            sead::Vector3f input = sead::Vector3f::zero;
            calcInputDirectionSnap2D(&input);
            sead::Vector3f up = sead::Vector3f::zero;
            al::calcUpDir(&up, mLiveActor);
            al::normalize(&up);
            if (input.dot(up) < -0.86603f)
                return true;
        }
    }

    const al::LiveActor* actor = mLiveActor;
    s32 port = PlayerFunction::getPlayerInputPort(actor);
    return PlayerInputFunction::isHoldSubAction(actor, port);
}

void PlayerInput::calcInputDirectionSnap2D(sead::Vector3f* input) const {
    if (_98)
        return;

    sead::Vector3f side(0.0f, 0.0f, 0.0f);
    al::calcSideDir(&side, mLiveActor);
    al::normalize(&side);
    calcMoveInputImpl(input, side, false, true, false);
    al::tryNormalizeOrZero(input);
}

bool PlayerInput::isHoldSpinCap() const {
    if (_98 || rs::isSeparatePlay(mLiveActor))
        return false;
    return PlayerInputFunction::isHoldAction(mLiveActor,
                                             PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isHoldCapAction() const {
    if (_98)
        return false;
    if (rs::isSeparatePlay(mLiveActor))
        return !PlayerInputFunction::isTriggerAction(mLiveActor, al::getPlayerControllerPort(1));
    return PlayerInputFunction::isHoldAction(mLiveActor,
                                             PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isHoldPoleClimbFast() const {
    return isHoldAction();
}

bool PlayerInput::isHoldWallCatchMoveFast() const {
    return isHoldAction();
}

bool PlayerInput::isHoldHackAction() const {
    return isHoldAction();
}

bool PlayerInput::isHoldHackJump() const {
    if (_98)
        return false;
    return PlayerInputFunction::isHoldJump(mLiveActor,
                                           PlayerFunction::getPlayerInputPort(mLiveActor));
}

// NON_MATCHING: behavior is recovered, but vector temporary scheduling differs; next source-level
// hypothesis is the original gravity/input expression form.
bool PlayerInput::isHoldEnterUpperDokan2D() const {
    if (_98)
        return false;
    const IUseDimension* dimension = mDimension;
    if (!dimension || !rs::is2D(dimension) || !rs::isIn2DArea(dimension))
        return false;

    sead::Vector3f input = sead::Vector3f::zero;
    calcInputDirectionSnap2D(&input);
    return input.dot(-al::getGravity(mLiveActor)) > 0.86603f;
}

// NON_MATCHING: behavior is recovered, but gravity/input temporary order differs; next source-level
// hypothesis is the original vector expression form.
bool PlayerInput::isHoldEnterSideDokan2D(const sead::Vector3f& dir) const {
    if (_98)
        return false;
    const IUseDimension* dimension = mDimension;
    if (!dimension || !rs::is2D(dimension) || !rs::isIn2DArea(dimension))
        return false;

    sead::Vector3f gravity = -al::getGravity(mLiveActor);
    sead::Vector3f input = sead::Vector3f::zero;
    calcMoveDirection(&input, gravity);
    return dir.dot(input) > 0.86603f;
}

void PlayerInput::calcMoveDirection(sead::Vector3f* input,
                                    const sead::Vector3f& gravity) const {
    if (_98 || !_18) {
        input->set(0.0f, 0.0f, 0.0f);
        return;
    }
    calcMoveInputImpl(input, gravity, false, false, false);
    al::tryNormalizeOrZero(input);
}

bool PlayerInput::isTriggerChange2D() const {
    if (_98)
        return false;
    u32 inputPort = PlayerFunction::getPlayerInputPort(mLiveActor);
    return al::isPadTriggerZL(inputPort) || al::isPadTriggerZR(inputPort);
}

bool PlayerInput::isTriggerChange3D() const {
    return isTriggerChange2D();
}

bool PlayerInput::isReleaseJump() const {
    if (_98)
        return false;
    return PlayerInputFunction::isReleaseJump(mLiveActor,
                                              PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isReleaseHackAction() const {
    if (_98)
        return false;
    return PlayerInputFunction::isReleaseAction(mLiveActor,
                                                PlayerFunction::getPlayerInputPort(mLiveActor));
}

bool PlayerInput::isReleaseHackJump() const {
    return isReleaseJump();
}

bool PlayerInput::isEnableDashInput() const {
    return isHoldAction();
}

bool PlayerInput::isSpinInput() const {
    return mSpinInputAnalyzer->getSpinDirection() != 0;
}

bool PlayerInput::isSpinClockwise() const {
    return mSpinInputAnalyzer->getSpinDirection() < 0;
}

bool PlayerInput::isSpinCounterClockwise() const {
    return mSpinInputAnalyzer->getSpinDirection() > 0;
}

void PlayerInput::calcMoveInput(sead::Vector3f* input, const sead::Vector3f& gravity) const {
    if (_98) {
        input->set(0.0f, 0.0f, 0.0f);
    } else {
        calcMoveInputImpl(input, gravity, false, false, false);
    }
}

// NON_MATCHING: behavior is recovered, but helper and vector lifetime order differ; next source-level
// hypothesis is the original invalid-input branch structure.
void PlayerInput::calc2DSnapJumpMoveInput(sead::Vector3f* input,
                                          const sead::Vector3f& gravity) const {
    if (_98) {
        input->set(0.0f, 0.0f, 0.0f);
        return;
    }

    if (!_cd) {
        sead::Vector3f move = sead::Vector3f::zero;
        sead::Vector3f side = sead::Vector3f::zero;
        al::calcSideDir(&side, mLiveActor);
        al::normalize(&side);
        calcMoveInputImpl(&move, side, false, true, false);
        al::tryNormalizeOrZero(&move);
        if (isInvalid2DSnapJumpMoveInput(move, mLiveActor, mPlayerCollision)) {
            input->set(0.0f, 0.0f, 0.0f);
            return;
        }
    }

    calcMoveInputImpl(input, gravity, false, false, false);
}

// NON_MATCHING: dot-product behavior is recovered, but camera-vector construction differs; next
// source-level hypothesis is the original matrix-axis helper expression.
bool PlayerInput::isPoleMoveInputReverseX() const {
    const al::LiveActor* actor = mLiveActor;
    const sead::Matrix34f* view = PlayerFunction::getPlayerViewMtx(actor);
    sead::Vector3f front = sead::Vector3f::zero;
    sead::Vector3f side = sead::Vector3f::zero;
    al::calcFrontDir(&front, actor);
    al::calcSideDir(&side, actor);

    sead::Vector3f cameraFront(-view->m[2][0], -view->m[2][1], -view->m[2][2]);
    f32 frontDot = front.dot(cameraFront);
    f32 sideDot = side.dot(cameraFront);
    return frontDot < 0.0f && sead::Mathf::abs(frontDot) > sead::Mathf::abs(sideDot);
}

const sead::Matrix34f* PlayerInput::getInputViewMtx() const {
    return PlayerFunction::getPlayerViewMtx(mLiveActor);
}

void PlayerInput::calcPoleMoveInput(sead::Vector2f* input) const {
    if (_98) {
        input->set(0.0f, 0.0f);
        return;
    }

    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    s32 stick = 0;
    if (rs::isSeparatePlay(mLiveActor)) {
        port = al::getPlayerControllerPort(0);
        stick = _8c;
    }
    sead::Vector2f move = PlayerInputFunction::getMoveInputStick(mLiveActor, port, stick);
    input->set(-move.x, move.y);
}

void PlayerInput::calcCapThrowInput(sead::Vector3f* input, const sead::Vector3f& gravity) const {
    calcMoveInput(input, gravity);
}

void PlayerInput::calcCapSeparateMoveInput(sead::Vector3f* input,
                                           const sead::Vector3f& gravity) const {
    if (_98) {
        input->set(0.0f, 0.0f, 0.0f);
    } else {
        calcMoveInputImpl(input, gravity, true, false, true);
    }
}

bool PlayerInput::isSameStickMove(const sead::Vector2f& dir, f32 angleDegree) const {
    if (_98 || !_18)
        return false;

    sead::Vector2f normalizedDir = dir;
    if (!al::tryNormalizeOrZero(&normalizedDir))
        return false;

    sead::Vector2f input = getStickMoveRaw();
    if (!al::tryNormalizeOrZero(&input))
        return false;

    return normalizedDir.dot(input) >= sead::Mathf::cos(sead::Mathf::deg2rad(angleDegree));
}

f32 PlayerInput::getRadiconInputSteeringValue() const {
    const al::LiveActor* actor = mLiveActor;
    s32 port = PlayerFunction::getPlayerInputPort(actor);
    return PlayerInputFunction::getRadiconInputSteeringValue(actor, port);
}

sead::Vector2f PlayerInput::getStickCameraRaw() const {
    if (_98)
        return sead::Vector2f::zero;
    return getCameraInputRaw();
}

// NON_MATCHING: camera selection behavior is recovered, but branch/register scheduling differs; next
// source-level hypothesis is the original separate-play fallback condition order.
sead::Vector2f PlayerInput::getCameraInputRaw() const {
    if (_98)
        return sead::Vector2f::zero;

    if (rs::isSeparatePlay(mLiveActor)) {
        s32 port1P = al::getPlayerControllerPort(0);
        s32 port2P = al::getPlayerControllerPort(1);
        sead::Vector2f input = PlayerInputFunction::getCameraMoveInput(
            mLiveActor, port1P, _8c, !_9a);
        if (al::isNearZero(input, 0.001f) &&
            !rs::isPlayerHackNoSeparateCameraInput(mLiveActor)) {
            input = PlayerInputFunction::getCameraMoveInput(mLiveActor, port2P, _90, true);
        }
        if (input.length() < 0.03f)
            input.set(sead::Vector2f::zero);
        return input;
    }

    sead::Vector2f input = PlayerInputFunction::getCameraMoveInput(
        mLiveActor, PlayerFunction::getPlayerInputPort(mLiveActor), 0, false);
    if (input.length() < 0.03f)
        input.set(sead::Vector2f::zero);
    return input;
}

sead::Vector2f PlayerInput::getStickCameraSubRaw() const {
    return getStickMoveRaw();
}

bool PlayerInput::isTriggerCameraReset() const {
    if (_98)
        return false;
    if (rs::isSeparatePlay(mLiveActor)) {
        return PlayerInputFunction::isTriggerSeparateCameraReset(
            mLiveActor, al::getPlayerControllerPort(0), _8c);
    }
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isTriggerCameraReset(mLiveActor, port);
}

bool PlayerInput::isTriggerCameraSubjective() const {
    if (_98 || rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isTriggerCameraSubjective(mLiveActor, port);
}

bool PlayerInput::isHoldBalloonSet() const {
    if (_98 || rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isHoldBalloonSet(mLiveActor, port);
}

bool PlayerInput::isHoldCameraZoom() const {
    if (_98 || rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isHoldCameraZoom(mLiveActor, port);
}

bool PlayerInput::isHoldCameraSnapShotZoomIn() const {
    if (_98 || rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isHoldCameraSnapShotZoomIn(mLiveActor, port);
}

bool PlayerInput::isHoldCameraSnapShotZoomOut() const {
    if (_98 || rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isHoldCameraSnapShotZoomOut(mLiveActor, port);
}

bool PlayerInput::isHoldCameraSnapShotRollLeft() const {
    if (_98 || rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isHoldCameraSnapShotRollLeft(mLiveActor, port);
}

bool PlayerInput::isHoldCameraSnapShotRollRight() const {
    if (_98 || rs::isSeparatePlay(mLiveActor))
        return false;
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    return PlayerInputFunction::isHoldCameraSnapShotRollRight(mLiveActor, port);
}

bool PlayerInput::isEnableShowTutorialInput() const {
    s32 port = PlayerFunction::getPlayerInputPort(mLiveActor);
    if ((al::isPadHold(port, PlayerInputFunction::getNoInputJudgeKeyMask(mLiveActor, port)) &&
         !rs::isTriggerUiPause(mLiveActor) &&
         !rs::isTriggerMapOpen(mLiveActor)) ||
        PlayerInputFunction::isInputLeftStickNoCameraMove(mLiveActor, port, 0.1f)) {
        return false;
    }
    return !mJoyPadAccelPoseAnalyzer1->isSwingAnyHand();
}

bool PlayerInput::isEnableRecoveryLifeInput() const {
    return isEnableShowTutorialInput();
}

// NON_MATCHING: returned direction behavior is recovered, but branch scheduling differs; next
// source-level hypothesis is the original hold/trigger condition grouping.
const sead::Vector2f& PlayerInput::getCapThrowDir() const {
    const sead::Vector2f* dir = &sead::Vector2f::zero;
    if (_98)
        return *dir;

    const al::LiveActor* actor = mLiveActor;
    al::JoyPadAccelPoseAnalyzer* analyzer = mJoyPadAccelPoseAnalyzer1;
    PlayerFunction::getPlayerInputPort(actor);
    if (analyzer->isSwingDoubleHandSameDir())
        return analyzer->getSwingDirDoubleHandSameDir();
    if (analyzer->isSwingLeftHand())
        return analyzer->getSwingLeftHandDir();
    if (analyzer->isSwingRightHand())
        dir = &analyzer->getSwingRightHandDir();
    return *dir;
}

// NON_MATCHING: returned direction behavior is recovered, but branch scheduling differs; next
// source-level hypothesis is the original swing-state condition grouping.
const sead::Vector2f& PlayerInput::getSwingThrowDir() const {
    const sead::Vector2f* dir = &sead::Vector2f::zero;
    if (_98)
        return *dir;

    const al::LiveActor* actor = mLiveActor;
    al::JoyPadAccelPoseAnalyzer* analyzer = mJoyPadAccelPoseAnalyzer1;
    PlayerFunction::getPlayerInputPort(actor);
    if (analyzer->isSwingLeftHand())
        return analyzer->getSwingLeftHandDir();
    if (analyzer->isSwingRightHand())
        dir = &analyzer->getSwingRightHandDir();
    return *dir;
}

bool PlayerInput::isSwingDirLeft() const {
    if (_98)
        return false;

    const al::LiveActor* actor = mLiveActor;
    al::JoyPadAccelPoseAnalyzer* analyzer = mJoyPadAccelPoseAnalyzer1;
    PlayerFunction::getPlayerInputPort(actor);
    const sead::Vector2f* dir = &sead::Vector2f::zero;
    if (analyzer->isSwingLeftHand())
        dir = &analyzer->getSwingLeftHandDir();
    else if (analyzer->isSwingRightHand())
        dir = &analyzer->getSwingRightHandDir();
    return dir->x < 0.0f;
}

bool PlayerInput::isSwingDirRight() const {
    if (_98)
        return false;

    const al::LiveActor* actor = mLiveActor;
    al::JoyPadAccelPoseAnalyzer* analyzer = mJoyPadAccelPoseAnalyzer1;
    PlayerFunction::getPlayerInputPort(actor);
    const sead::Vector2f* dir = &sead::Vector2f::zero;
    if (analyzer->isSwingLeftHand())
        dir = &analyzer->getSwingLeftHandDir();
    else if (analyzer->isSwingRightHand())
        dir = &analyzer->getSwingRightHandDir();
    return dir->x > 0.0f;
}

bool PlayerInput::isThrowTypeLeftRight(const sead::Vector2f& dir) const {
    return !al::isNearZero(dir, 0.001f);
}

bool PlayerInput::isThrowTypeSpiral(const sead::Vector2f& dir) const {
    if (al::isNearZero(dir, 0.001))
        return false;
    f32 absX = (dir.x > 0 ? dir.x : -dir.x);
    f32 absY = (dir.y > 0 ? dir.y : -dir.y);
    return absX > absY;
}

bool PlayerInput::isThrowTypeRolling(const sead::Vector2f& dir) const {
    if (al::isNearZero(dir, 0.001))
        return !al::isNearZero(dir.y, 0.001);
    f32 absX = (dir.x > 0 ? dir.x : -dir.x);
    f32 absY = (dir.y > 0 ? dir.y : -dir.y);
    return !(absX > absY) && !al::isNearZero(dir.y, 0.001);
}

bool PlayerInput::isEnableConsiderCapThrowDoubleSwing() const {
    return al::getPadAccelerationDeviceNum(PlayerFunction::getPlayerInputPort(mLiveActor)) == 1;
}

f32 PlayerInput::getPoseRotZDegreeLeft() const {
    return mJoyPadAccelPoseAnalyzer1->getPoseRotZDegreeLeft();
}

f32 PlayerInput::getPoseRotZDegreeRight() const {
    return mJoyPadAccelPoseAnalyzer1->getPoseRotZDegreeRight();
}

bool PlayerInput::isTriggerSwingLeftHand() const {
    return !_98 && mJoyPadAccelPoseAnalyzer1->isSwingLeftHand();
}

bool PlayerInput::isTriggerSwingRightHand() const {
    return !_98 && mJoyPadAccelPoseAnalyzer1->isSwingRightHand();
}

bool PlayerInput::isTriggerSwingDoubleHand() const {
    if (_98)
        return false;
    return mJoyPadAccelPoseAnalyzer1->isSwingDoubleHand();
}

bool PlayerInput::isTriggerSwingDoubleHandReverseDir() const {
    if (_98)
        return false;
    return mJoyPadAccelPoseAnalyzer1->isSwingDoubleHandReverseDir();
}

bool PlayerInput::isTriggerSwingDoubleHandReverseInsideDir() const {
    if (_98)
        return false;
    return mJoyPadAccelPoseAnalyzer1->isSwingDoubleHandReverseInsideDir();
}

bool PlayerInput::isTriggerSwingDoubleHandReverseOutsideDir() const {
    if (_98)
        return false;
    return mJoyPadAccelPoseAnalyzer1->isSwingDoubleHandReverseOutsideDir();
}

const sead::Vector2f& PlayerInput::getSwingVelLeftHand() const {
    if (_98)
        return sead::Vector2f::zero;
    return mJoyPadAccelPoseAnalyzer1->getSwingVelLeftHand();
}

const sead::Vector2f& PlayerInput::getSwingVelRightHand() const {
    if (_98)
        return sead::Vector2f::zero;
    return mJoyPadAccelPoseAnalyzer1->getSwingVelRightHand();
}

const sead::Vector2f& PlayerInput::getSwingLeftHandDir() const {
    if (_98)
        return sead::Vector2f::zero;
    return mJoyPadAccelPoseAnalyzer1->getSwingLeftHandDir();
}

const sead::Vector2f& PlayerInput::getSwingRightHandDir() const {
    if (_98)
        return sead::Vector2f::zero;
    return mJoyPadAccelPoseAnalyzer1->getSwingRightHandDir();
}

void PlayerInput::resetHoldInfo3D() {
    _9c = false;
    _a0.set(0.0f, 0.0f);
    _a8.set(0.0f, 0.0f, 0.0f);
    _b4.set(0.0f, 0.0f, 0.0f);
    _c0.set(0.0f, 0.0f, 0.0f);
}

// NON_MATCHING: target emits a framed BL to memset for the 0x70-byte reset; void source through sead::MemUtil tail-calls instead. Next hypothesis: recover the original non-inline zeroing wrapper without changing the proven void ABI.
void PlayerInput::resetHoldInfo2D() {
    _cd = false;
    sead::MemUtil::fillZero(&_d0, 0x70);
}

