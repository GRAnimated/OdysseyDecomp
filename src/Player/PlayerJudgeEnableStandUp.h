#pragma once

#include "Player/IJudge.h"

class IUsePlayerCeilingCheck;
class IUsePlayerCollision;

class PlayerJudgeEnableStandUp : public IJudge {
public:
    PlayerJudgeEnableStandUp(const IUsePlayerCollision* collider,
                             const IUsePlayerCeilingCheck* ceilingChecker);

    bool judge() const override;

    void reset() override {}
    void update() override {}

private:
    const IUsePlayerCollision* mCollider;
    const IUsePlayerCeilingCheck* mCeilingChecker;
};

static_assert(sizeof(PlayerJudgeEnableStandUp) == 0x18);
