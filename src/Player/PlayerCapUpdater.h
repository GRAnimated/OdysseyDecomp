#pragma once

#include "Library/Execute/IUseExecutor.h"

namespace al {
class ExecuteDirector;
}
class HackCap;
class PlayerModelChangerHakoniwa;

class PlayerCapUpdater : public al::IUseExecutor {
public:
    PlayerCapUpdater(HackCap*, PlayerModelChangerHakoniwa*);

    void init(al::ExecuteDirector*);
    void execute() override;

private:
    HackCap* mHackCap;
    PlayerModelChangerHakoniwa* mModelChanger;
};

static_assert(sizeof(PlayerCapUpdater) == 0x18);
