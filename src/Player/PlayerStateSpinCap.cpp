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

bool PlayerStateSpinCap::update() {
    if (!mJudgeWaterSurfaceRun->mIsEnable && mCounterForceRun->getCounter() >= 1)
        mJudgeWaterSurfaceRun->mIsEnable = true;

    rs::updateJudge(mJudgeWaterSurfaceRun);
    PlayerActionGroundMoveControl* groundMoveControl = mGroundMoveControl;
    const s32 counter = mCounterForceRun->getCounter();
    const f32 speed = mCounterForceRun->getSpeed();
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
