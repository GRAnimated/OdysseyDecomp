#pragma once

#include <basis/seadTypes.h>

#include "Library/HostIO/HioNode.h"

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class IUsePlayerCollision;

class HackerJudgeNormalFall : public al::HioNode, public IJudge {
public:
    HackerJudgeNormalFall(const al::LiveActor* parent, s32 fallTime);
    void reset() override;
    void update() override;
    bool judge() const override;

    void setPlayerCollision(IUsePlayerCollision* playerCollision) {
        mPlayerCollision = playerCollision;
    }

    void setFallTime(s32 fallTime) { mFallTime = fallTime; }

private:
    const al::LiveActor* mActor;
    s32 mFallTime;
    s32 mAirTime = 0;
    IUsePlayerCollision* mPlayerCollision = nullptr;
};

static_assert(sizeof(HackerJudgeNormalFall) == 0x20);
