#pragma once

#include "Library/Execute/IUseExecutor.h"

namespace al {
class ExecuteDirector;
}
class HackCap;
class PlayerModelChangerHakoniwa;

class PlayerCapUpdater : public al::IUseExecutor {
public:
    PlayerCapUpdater(HackCap* hackCap, PlayerModelChangerHakoniwa* modelChanger);

    void init(al::ExecuteDirector* executeDirector);
    void execute() override;

private:
    HackCap* mHackCap;
    PlayerModelChangerHakoniwa* mModelChanger;
};

