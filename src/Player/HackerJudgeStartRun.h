#pragma once

#include <basis/seadTypes.h>

#include "Util/HackerJudge.h"

namespace al {
class LiveActor;
}

class IUsePlayerCollision;
class IUsePlayerHack;

class HackerJudgeStartRun : public HackerJudge {
public:
    HackerJudgeStartRun(const al::LiveActor* parent, IUsePlayerHack** hacker);
    bool judge() const override;
    void reset() override;
    void update() override;

    void setPlayerCollision(IUsePlayerCollision* playerCollision) {
        mPlayerCollision = playerCollision;
    }

private:
    const al::LiveActor* mParent;
    const s32* _18;
    const IUsePlayerCollision* mPlayerCollision;
    f32 _28;
};

static_assert(sizeof(HackerJudgeStartRun) == 0x30);
