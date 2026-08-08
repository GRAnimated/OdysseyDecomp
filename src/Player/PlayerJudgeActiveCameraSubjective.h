#pragma once

#include "Player/IJudge.h"

class PlayerStateCameraSubjective;

class PlayerJudgeActiveCameraSubjective : public IJudge {
public:
    PlayerJudgeActiveCameraSubjective(const PlayerStateCameraSubjective* stateCameraSubjective);

    bool judge() const override;
    void reset() override {}
    void update() override {}

private:
    const PlayerStateCameraSubjective* mStateCameraSubjective;
};

static_assert(sizeof(PlayerJudgeActiveCameraSubjective) == 0x10);
