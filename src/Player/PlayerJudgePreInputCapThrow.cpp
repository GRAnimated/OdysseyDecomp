#include "Player/PlayerJudgePreInputCapThrow.h"

#include <math/seadMathCalcCommon.h>

#include "Player/HackCap.h"
#include "Player/PlayerCarryKeeper.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"

PlayerJudgePreInputCapThrow::PlayerJudgePreInputCapThrow(const PlayerConst* playerConst,
                                                         const PlayerInput* input,
                                                         const PlayerCarryKeeper* carryKeeper,
                                                         const HackCap* hackCap)
    : mConst(playerConst), mInput(input), mCarryKeeper(carryKeeper), mHackCap(hackCap),
      mRemainFrame(0), mRemainFrameDouble(0) {
    clearInfo(&mRecordedInfo);
    clearInfo(&mCurrentInfo);
}

void PlayerJudgePreInputCapThrow::recordJudgeAndReset() {
    mRecordedInfo = mCurrentInfo;
    reset();
}

void PlayerJudgePreInputCapThrow::recordSeparateJudge() {
    mRecordedInfo.isCooperate = false;
    mRecordedInfo.doubleThrowDir.set(0.0f, 0.0f);
    mRecordedInfo.throwDir.set(0.0f, 0.0f);
    mRecordedInfo.throwType = 1;
}

void PlayerJudgePreInputCapThrow::recordCooperateAndReset() {
    mRecordedInfo = mCurrentInfo;
    mRecordedInfo.isCooperate = true;
    reset();
}

void PlayerJudgePreInputCapThrow::reset() {
    mRemainFrame = 0;
    mRemainFrameDouble = 0;
}

void PlayerJudgePreInputCapThrow::update() {
    mRemainFrame = sead::Mathi::clampMin(mRemainFrame - 1, 0);
    mRemainFrameDouble = sead::Mathi::clampMin(mRemainFrameDouble - 1, 0);

    if (mInput->isTriggerSpinCap()) {
        if (mInput->isTriggerCapDoubleHandThrow()) {
            mCurrentInfo.isCooperate = false;
            mCurrentInfo.doubleThrowDir.set(0.0f, 0.0f);
            mCurrentInfo.throwDir.set(0.0f, 0.0f);
            mCurrentInfo.throwType = 4;
            mCurrentInfo.doubleThrowDir = mInput->getCapThrowDir();
            mRemainFrame = mConst->getPreInputFrameCapThrow();
            mRemainFrameDouble = mConst->getPreInputFrameCapThrow();
        } else if (mInput->isTriggerCapSingleHandThrow()) {
            if (mRemainFrameDouble <= 0) {
                mCurrentInfo = {0, sead::Vector2f(0.0f, 0.0f),
                                sead::Vector2f(0.0f, 0.0f), false};
                mCurrentInfo.throwType = mInput->isTriggerSwingRightHand() ? 3 : 2;
                mCurrentInfo.throwDir = mInput->getCapThrowDir();
                mRemainFrame = mConst->getPreInputFrameCapThrow();

                if (mInput->isEnableConsiderCapThrowDoubleSwing() &&
                    (mInput->isThrowTypeSpiral(mCurrentInfo.throwDir) || mCurrentInfo.throwDir.y > 0.0f)) {
                    mCurrentInfo.throwType = 4;
                    mCurrentInfo.doubleThrowDir = mCurrentInfo.throwDir;
                    mCurrentInfo.throwDir.set(0.0f, 0.0f);
                    mRemainFrameDouble = mConst->getPreInputFrameCapThrow();
                }
            }
        } else {
            mCurrentInfo.isCooperate = false;
            mCurrentInfo.doubleThrowDir.set(0.0f, 0.0f);
            mCurrentInfo.throwDir.set(0.0f, 0.0f);
            mCurrentInfo.throwType = 1;
            mRemainFrame = mConst->getPreInputFrameCapThrow();
        }
    }

    if (mCarryKeeper->isThrow())
        mRemainFrame = 0;
    if (mHackCap->isRequestableReturn())
        mRemainFrame = 0;
    if (!mHackCap->isEnablePreInput())
        mRemainFrame = 0;
}

bool PlayerJudgePreInputCapThrow::judge() const {
    if (mCarryKeeper->isThrow())
        return false;
    if (mInput->isTriggerSpinCap())
        return true;
    return mRemainFrame > 0;
}
