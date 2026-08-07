#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class HackCap;
class IUsePlayerCollision;
class PlayerInput;

class HackCapJudgePreInputHoveringJump : public IJudge {
public:
    HackCapJudgePreInputHoveringJump(const al::LiveActor*, const IUsePlayerCollision*,
                                     const HackCap*, const PlayerInput*);

    void reset() override;
    void update() override;
    bool judge() const override;

    void setDisabled(bool isDisabled) { mIsDisabled = isDisabled; }

private:
    const al::LiveActor* mPlayer;
    const IUsePlayerCollision* mCollision;
    const HackCap* mHackCap;
    const PlayerInput* mInput;
    s32 mRemainFrame = 0;
    bool mIsDisabled = false;
};

static_assert(sizeof(HackCapJudgePreInputHoveringJump) == 0x30);
