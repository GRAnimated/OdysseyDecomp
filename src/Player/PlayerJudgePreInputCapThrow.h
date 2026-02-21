#pragma once

#include <math/seadVector.h>

#include "Player/IJudge.h"

class HackCap;
class PlayerCarryKeeper;
class PlayerConst;
class PlayerInput;

class PlayerJudgePreInputCapThrow : public IJudge {
public:
    PlayerJudgePreInputCapThrow(const PlayerConst* pConst, const PlayerInput* input,
                                const PlayerCarryKeeper* carryKeeper, const HackCap* hackCap);
    void reset() override;
    void update() override;
    bool judge() const override;

    void recordJudgeAndReset();
    void recordSeparateJudge();
    void recordCooperateAndReset();

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const PlayerCarryKeeper* mCarryKeeper;
    const HackCap* mHackCap;
    s32 mPreInputFrame = 0;
    s32 mPreInputFrameSingle = 0;
    s32 mThrowType = 0;
    sead::Vector2f mCapThrowDir = {0.0f, 0.0f};
    sead::Vector2f mCooperateCapThrowDir = {0.0f, 0.0f};
    bool mIsCooperate = false;
    s32 mRecordedThrowType = 0;
    sead::Vector2f mRecordedCapThrowDir = {0.0f, 0.0f};
    sead::Vector2f mRecordedCooperateCapThrowDir = {0.0f, 0.0f};
    bool mIsRecordedCooperate = false;
};

static_assert(sizeof(PlayerJudgePreInputCapThrow) == 0x60);
