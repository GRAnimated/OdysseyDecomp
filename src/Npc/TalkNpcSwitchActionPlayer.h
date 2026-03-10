#pragma once

#include "Library/Placement/PlacementFunction.h"
#include "Library/Stage/IUseStageSwitch.h"
#include "Library/Stage/StageSwitchKeeper.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"

#include "Npc/RandomActionUpdater.h"

namespace al {
class StageSwitchDirector;
}  // namespace al

class TalkNpcSwitchActionPlayer : public al::IUseStageSwitch {
public:
    TalkNpcSwitchActionPlayer(al::StageSwitchDirector* switchDirector, RandomActionUpdater* updater,
                              const al::PlacementInfo& placementInfo)
        : mActionUpdater(updater) {
        al::initStageSwitch(this, switchDirector, placementInfo);
        al::FunctorV0M<TalkNpcSwitchActionPlayer*, void (TalkNpcSwitchActionPlayer::*)()> functor(
            this, &TalkNpcSwitchActionPlayer::listenStageSwitchStartActionOn);
        al::listenStageSwitchOn(this, "SwitchActionStart", functor);
        al::getStringArg(&mActionName, placementInfo, "ActionName");
    }

    const char* getName() const override {
        return u8"スイッチONでトークNPCのアクションを再生する機能";
    }

    al::StageSwitchKeeper* getStageSwitchKeeper() const override { return mStageSwitchKeeper; }

    void initStageSwitchKeeper() override { mStageSwitchKeeper = new al::StageSwitchKeeper(); }

    void listenStageSwitchStartActionOn() { mActionUpdater->startActionOneTime(mActionName); }

private:
    al::StageSwitchKeeper* mStageSwitchKeeper = nullptr;
    RandomActionUpdater* mActionUpdater = nullptr;
    const char* mActionName = nullptr;
};

static_assert(sizeof(TalkNpcSwitchActionPlayer) == 0x20);
