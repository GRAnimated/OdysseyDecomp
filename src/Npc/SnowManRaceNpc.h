#pragma once

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

class ChangeStageInfo;
class NpcStateReaction;
class TalkNpcActionAnimInfo;
class TalkNpcParam;

namespace al {
class CameraTicket;
class EventFlowExecutor;
}  // namespace al

class SnowManRaceNpc : public al::LiveActor,
                       public al::IEventFlowEventReceiver,
                       public al::IEventFlowQueryJudge {
public:
    SnowManRaceNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    void control() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) override;
    bool receiveEvent(const al::EventFlowEventData* event) override;
    const char* judgeQuery(const char* query) const override;

    void setHackKeepTalkDemo();
    void updateRandomAction();
    void exeWaitNoPlayerHack();
    void exeWaitPlayerHack();
    void exeReaction();
    void exeRaceResultWin();
    void exeRaceResultWinWait();
    void exeRaceResultFirst();
    void exeRaceResultSecond();
    void exeRaceResultThird();
    void exeGetPrize();
    void exeGetPrizeEnd();
    void exeRaceResultLose();
    void exeRaceResultCancel();

private:
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    NpcStateReaction* mReactionState = nullptr;
    ChangeStageInfo* mChangeStageInfo = nullptr;
    ChangeStageInfo* mChangeStageInfoSecond = nullptr;
    al::LiveActor* mYukimaruActor = nullptr;
    al::CameraTicket* mDemoCamera = nullptr;
    al::CameraTicket* mEntranceCamera = nullptr;
    s32 mPrizeCount = 0;
    s32 mPlaceScenario = 0;
    s32 mRaceResult = 5;
    bool mIsHackStarted = true;
    bool mIsLv1ClearFlag = false;
    TalkNpcParam* mTalkNpcParam = nullptr;
    TalkNpcActionAnimInfo* mActionAnimInfo = nullptr;
    s32 mTimer = 0;
    bool mIsFarFromPlayer = true;
};

static_assert(sizeof(SnowManRaceNpc) == 0x178);
