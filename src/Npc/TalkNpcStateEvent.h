#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class EventFlowExecutor;
class HitSensor;
class SensorMsg;
}  // namespace al

class ActorStateReactionBase;
class BirdMtxGlideCtrl;
class RandomActionUpdater;
class TalkNpcActionAnimInfo;
class TalkNpcParam;
class TalkNpcPartialAnimCtrl;

class TalkNpcStateEvent : public al::ActorStateBase {
public:
    TalkNpcStateEvent(const char* name, al::LiveActor* actor, const TalkNpcParam* param,
                      al::EventFlowExecutor* executor, s32);

    void initStateReaction(ActorStateReactionBase* reaction);
    void initAfterPlacement();
    void appear() override;
    void kill() override;
    bool update() override;
    bool trySwitchEventEntry(const char* entryName);
    bool trySwitchEventEntryAndUpdate(const char* entryName);
    void switchEventEntry(const char* entryName);
    bool tryStartReaction(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self);
    bool tryStartReactionWithoutTrample(const al::SensorMsg* msg, al::HitSensor* other,
                                        al::HitSensor* self);
    bool isCapReaction() const;
    bool isScare() const;

    void exeActive();
    void endActive();
    void exeReaction();
    void exeScareStart();
    void exeScare();
    void exeScareEnd();
    void exeDemo();
    void exeStopByTraffic();
    void exeBalloonAction();

    void setAnimInfo(TalkNpcActionAnimInfo* info) { mAnimInfo = info; }
    void setPartialAnimCtrl(TalkNpcPartialAnimCtrl* ctrl) { mPartialAnimCtrl = ctrl; }
    void setRandomActionUpdater(RandomActionUpdater* updater) { mRandomActionUpdater = updater; }
    void setGlideCtrl(BirdMtxGlideCtrl* ctrl) { mGlideCtrl = ctrl; }
    void setIsTrafficNpc(bool v) { _5d = v; }
    void setIsHackWait(bool v) { mIsHackWait = v; }

    const TalkNpcParam* mTalkNpcParam;
    al::EventFlowExecutor* mEventFlowExecutor;
    ActorStateReactionBase* mStateReaction = nullptr;
    TalkNpcActionAnimInfo* mAnimInfo = nullptr;
    TalkNpcPartialAnimCtrl* mPartialAnimCtrl = nullptr;
    RandomActionUpdater* mRandomActionUpdater = nullptr;
    BirdMtxGlideCtrl* mGlideCtrl = nullptr;
    s32 _58 = 0;
    bool _5c = false;
    bool _5d = false;
    bool mIsHackWait = false;

private:
    static bool tryStartScare(TalkNpcStateEvent* state, al::EventFlowExecutor* executor,
                              TalkNpcPartialAnimCtrl* partialAnimCtrl);
};

static_assert(sizeof(TalkNpcStateEvent) == 0x60);
