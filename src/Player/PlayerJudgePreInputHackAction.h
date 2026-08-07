#pragma once

#include <basis/seadTypes.h>

#include "Library/HostIO/HioNode.h"

#include "Player/IJudge.h"

class PlayerConst;
class PlayerInput;

class PlayerJudgePreInputHackAction : public al::HioNode, public IJudge {
public:
    PlayerJudgePreInputHackAction(const PlayerConst* playerConst, const PlayerInput* input);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    s32 mRemainFrame = 0;
};

static_assert(sizeof(PlayerJudgePreInputHackAction) == 0x20);
