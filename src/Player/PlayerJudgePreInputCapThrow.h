#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/IJudge.h"

class HackCap;
class PlayerCarryKeeper;
class PlayerConst;
class PlayerInput;

class PlayerJudgePreInputCapThrow : public IJudge {
public:
    PlayerJudgePreInputCapThrow(const PlayerConst* playerConst, const PlayerInput* input,
                                const PlayerCarryKeeper* carryKeeper, const HackCap* hackCap);

    void reset() override;
    void update() override;
    bool judge() const override;

    void recordJudgeAndReset();
    void recordSeparateJudge();
    void recordCooperateAndReset();

    s32 getRecordedThrowType() const { return mRecordedInfo.throwType; }
    const sead::Vector2f& getRecordedThrowDir() const { return mRecordedInfo.throwDir; }
    const sead::Vector2f& getRecordedDoubleThrowDir() const { return mRecordedInfo.doubleThrowDir; }
    bool isRecordedCooperate() const { return mRecordedInfo.isCooperate; }

private:
    struct CapThrowInfo {
        s32 throwType;
        sead::Vector2f throwDir;
        sead::Vector2f doubleThrowDir;
        bool isCooperate;
    };
    static_assert(sizeof(CapThrowInfo) == 0x18);

    static void clearInfo(CapThrowInfo* info) {
        *info = {0, sead::Vector2f(0.0f, 0.0f), sead::Vector2f(0.0f, 0.0f), false};
    }

    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const PlayerCarryKeeper* mCarryKeeper;
    const HackCap* mHackCap;
    s32 mRemainFrame;
    s32 mRemainFrameDouble;
    CapThrowInfo mCurrentInfo;
    CapThrowInfo mRecordedInfo;
};


static_assert(sizeof(PlayerJudgePreInputCapThrow) == 0x60);
