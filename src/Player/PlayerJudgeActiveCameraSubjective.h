#pragma once

#include "Player/IJudge.h"

class PlayerStateCameraSubjective;

class PlayerJudgeActiveCameraSubjective : public IJudge {
public:
    PlayerJudgeActiveCameraSubjective(const PlayerStateCameraSubjective* stateCameraSubjective);

    void reset() override {}
    void update() override {}
    bool judge() const override;

private:
    const PlayerStateCameraSubjective* mStateCameraSubjective;
};

static_assert(sizeof(PlayerJudgeActiveCameraSubjective) == 0x10);
