#pragma once

#include "Library/Stage/IUseStageSwitch.h"

namespace al {
struct ActorInitInfo;
class PlacementInfo;
class StageSwitchDirector;
class StageSwitchKeeper;
}  // namespace al

class RandomActionUpdater;
class TalkNpcActionAnimInfo;
class TalkNpcParam;

class TalkNpcSwitchWaitActionObj : public al::IUseStageSwitch {
public:
    static TalkNpcSwitchWaitActionObj* tryCreate(const TalkNpcParam* param,
                                                 TalkNpcActionAnimInfo* animInfo,
                                                 RandomActionUpdater* updater,
                                                 const al::ActorInitInfo& initInfo);

    TalkNpcSwitchWaitActionObj(al::StageSwitchDirector* switchDirector, const TalkNpcParam* param,
                               TalkNpcActionAnimInfo* animInfo, RandomActionUpdater* updater,
                               const al::PlacementInfo& placementInfo);

    const char* getName() const override {
        return u8"スイッチONでトークNPCのアクションを再生する機能";
    }

    al::StageSwitchKeeper* getStageSwitchKeeper() const override { return mStageSwitchKeeper; }

    void initStageSwitchKeeper() override;

    void listenStageSwitchStartWaitActionSwitchOn();
    void listenStageSwitchStartWaitActionSwitchOff();

private:
    al::StageSwitchKeeper* mStageSwitchKeeper = nullptr;
    const TalkNpcParam* mParam = nullptr;
    TalkNpcActionAnimInfo* mAnimInfo = nullptr;
    RandomActionUpdater* mActionUpdater = nullptr;
    const char* mWaitActionName = nullptr;
};

static_assert(sizeof(TalkNpcSwitchWaitActionObj) == 0x30);
