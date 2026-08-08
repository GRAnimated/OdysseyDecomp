#include "Player/PlayerStateSpinCap.h"

#include "Player/PlayerJointParamCapThrow.h"
#include "Player/PlayerActionGroundMoveControl.h"
#include "Player/PlayerCounterForceRun.h"
#include "Player/PlayerJudgeWaterSurfaceRun.h"
#include "Player/PlayerSpinCapAttack.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"

void PlayerStateSpinCap::kill() {
    al::NerveStateBase::kill();
    if (mSpinCapAttack->tryCancelCapState(mAnimator))
        resetJoint();
}

// NON_MATCHING: current full-build body is 112 bytes vs target 104; the clean accessor/local form introduces an extra register-move/load schedule around the adjacent force-run counter fields. Next hypothesis is the original direct-field/lifetime form that preserves the target LDP count/speed sequence without duplicating code.
bool PlayerStateSpinCap::update() {
    if (!mJudgeWaterSurfaceRun->mIsEnable && mCounterForceRun->getCounter() >= 1)
        mJudgeWaterSurfaceRun->mIsEnable = true;

    rs::updateJudge(mJudgeWaterSurfaceRun);
    const PlayerCounterForceRun* counterForceRun = mCounterForceRun;
    const s32 counter = counterForceRun->getCounter();
    const f32 speed = counterForceRun->getSpeed();
    PlayerActionGroundMoveControl* groundMoveControl = mGroundMoveControl;
    groundMoveControl->mIsForceRunCtrlActive = counter > 0;
    groundMoveControl->_a0 = speed;
    return al::NerveStateBase::update();
}

void PlayerStateSpinCap::control() {
    if (rs::isOnGround(mActor, mCollision))
        mIsOnGround = false;
}

bool PlayerStateSpinCap::noticeInWater() {
    if (_98)
        return false;

    _98 = true;
    return true;
}

void PlayerStateSpinCap::resetJoint() {
    mCapThrowJoint->isEnd = true;
}

PlayerStateSpinCap::~PlayerStateSpinCap() = default;
