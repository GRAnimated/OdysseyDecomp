#include "Player/PlayerJudgePreInputCapThrow.h"

#include <math/seadMathCalcCommon.h>

#include "Player/HackCap.h"
#include "Player/PlayerCarryKeeper.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"

// NON_MATCHING: zero-init store scheduling differs (stur xzr vs strb wzr ordering)
PlayerJudgePreInputCapThrow::PlayerJudgePreInputCapThrow(const PlayerConst* pConst,
                                                        const PlayerInput* input,
                                                        const PlayerCarryKeeper* carryKeeper,
                                                        const HackCap* hackCap)
    : mConst(pConst), mInput(input), mCarryKeeper(carryKeeper), mHackCap(hackCap) {}

void PlayerJudgePreInputCapThrow::reset() {
    mPreInputFrame = 0;
    mPreInputFrameSingle = 0;
}

// NON_MATCHING: spiral upgrade branch (isThrowTypeSpiral + dir.y > 0) layout differs; missing shared
// LABEL_27 tail block; equivalent logic
void PlayerJudgePreInputCapThrow::update() {
    mPreInputFrame = sead::Mathi::clampMin(mPreInputFrame - 1, 0);
    mPreInputFrameSingle = sead::Mathi::clampMin(mPreInputFrameSingle - 1, 0);

    if (mInput->isTriggerSpinCap()) {
        if (mInput->isTriggerCapDoubleHandThrow()) {
            mIsCooperate = false;
            mCooperateCapThrowDir = {0.0f, 0.0f};
            mCapThrowDir = {0.0f, 0.0f};
            mThrowType = 4;
            mCooperateCapThrowDir = mInput->getCapThrowDir();
            mPreInputFrame = mConst->getPreInputFrameCapThrow();
            mPreInputFrameSingle = mConst->getPreInputFrameCapThrow();
        } else if (mInput->isTriggerCapSingleHandThrow()) {
            if (mPreInputFrameSingle <= 0) {
                mIsCooperate = false;
                mThrowType = 0;
                mCapThrowDir = {0.0f, 0.0f};
                mCooperateCapThrowDir = {0.0f, 0.0f};
                mThrowType = mInput->isTriggerSwingRightHand() ? 3 : 2;
                mCapThrowDir = mInput->getCapThrowDir();
                mPreInputFrame = mConst->getPreInputFrameCapThrow();
                if (mInput->isEnableConsiderCapThrowDoubleSwing()) {
                    bool is_spiral = mInput->isThrowTypeSpiral(mCapThrowDir);
                    if (!is_spiral)
                        is_spiral = mCapThrowDir.y > 0.0f;
                    if (is_spiral) {
                        sead::Vector2f old_dir = mCapThrowDir;
                        mThrowType = 4;
                        mCapThrowDir = {0.0f, 0.0f};
                        mCooperateCapThrowDir = old_dir;
                        mPreInputFrameSingle = mConst->getPreInputFrameCapThrow();
                    }
                }
            }
        } else {
            mIsCooperate = false;
            mCooperateCapThrowDir = {0.0f, 0.0f};
            mCapThrowDir = {0.0f, 0.0f};
            mThrowType = 1;
            mPreInputFrame = mConst->getPreInputFrameCapThrow();
        }
    }

    if (mCarryKeeper->isThrow())
        mPreInputFrame = 0;
    if (mHackCap->isRequestableReturn())
        mPreInputFrame = 0;
    if (!mHackCap->isEnablePreInput())
        mPreInputFrame = 0;
}

bool PlayerJudgePreInputCapThrow::judge() const {
    if (mCarryKeeper->isThrow())
        return false;
    if (mInput->isTriggerSpinCap())
        return true;
    return mPreInputFrame > 0;
}

// NON_MATCHING: ldr x8 + lsr x9 instead of ldp w8, w9 for loading mThrowType+mCapThrowDir.x pair
void PlayerJudgePreInputCapThrow::recordJudgeAndReset() {
    mRecordedThrowType = mThrowType;
    mRecordedCapThrowDir = mCapThrowDir;
    mRecordedCooperateCapThrowDir.x = mCooperateCapThrowDir.x;
    mRecordedCooperateCapThrowDir.y = mCooperateCapThrowDir.y;
    mIsRecordedCooperate = mIsCooperate;
    reset();
}

void PlayerJudgePreInputCapThrow::recordSeparateJudge() {
    mIsRecordedCooperate = false;
    mRecordedCooperateCapThrowDir = {0.0f, 0.0f};
    mRecordedCapThrowDir = {0.0f, 0.0f};
    mRecordedThrowType = 1;
}

// NON_MATCHING: ldr x8 + lsr x9 instead of ldp w8, w9 for loading mThrowType+mCapThrowDir.x pair
void PlayerJudgePreInputCapThrow::recordCooperateAndReset() {
    mRecordedThrowType = mThrowType;
    mRecordedCapThrowDir = mCapThrowDir;
    mRecordedCooperateCapThrowDir.x = mCooperateCapThrowDir.x;
    mRecordedCooperateCapThrowDir.y = mCooperateCapThrowDir.y;
    mIsRecordedCooperate = true;
    reset();
}
