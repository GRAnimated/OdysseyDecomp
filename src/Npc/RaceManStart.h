#pragma once

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
class AddDemoInfo;
class EventFlowExecutor;
class PlacementId;
}  // namespace al

class NpcJointLookAtController;
class NpcStateReaction;
class TalkNpcCap;
class TalkNpcParam;

class RaceManStart : public al::LiveActor,
                     public al::IEventFlowEventReceiver,
                     public al::IEventFlowQueryJudge {
public:
    RaceManStart(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;
    void control() override;

    bool receiveEvent(const al::EventFlowEventData* data) override;
    const char* judgeQuery(const char* query) const override;

    void exeWait();
    void exeRaceRank1();
    void exeRaceRankCoin();
    void exeRaceRankLose();
    void exeEventPrize();
    void exeEventPrizeCoin();
    void exeAppearCoin();
    void exeAppearCoinEnd();
    void exeEventRetry();
    void exeEventLose();
    void exeTalk();
    void exeEnd();
    void exeReaction();
    void exeRaceRank2();
    void exeRaceRank3();

private:
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    const char* mStageNameLv1 = nullptr;
    s32 mScenarioNoLv1 = 4;
    const char* mStartPosLv1 = nullptr;
    const char* mStageNameLv2 = nullptr;
    s32 mScenarioNoLv2 = 5;
    const char* mStartPosLv2 = nullptr;
    s32 mEndRaceLv = -1;
    s32 mNextRaceLv = -1;
    s32 mRaceResult = -1;
    s32 mCoinCount = 0;
    s32 mCoinMax = 0;
    bool mIsPlayFirstTimeThisScene = false;
    bool mIsPlayFirstTimeThisSceneLv2 = false;
    al::PlacementId* mShineActorId = nullptr;
    al::PlacementId* mShineActorMoonLockId = nullptr;
    bool mIsRaceCancel = false;
    NpcStateReaction* mNpcStateReaction = nullptr;
    TalkNpcParam* mTalkNpcParam = nullptr;
    NpcJointLookAtController* mJointLookAtController = nullptr;
    TalkNpcCap* mTalkNpcCap = nullptr;
    bool mIsEventStarted = false;
    al::AddDemoInfo* mAddDemoInfo = nullptr;
};

static_assert(sizeof(RaceManStart) == 0x1B0);
