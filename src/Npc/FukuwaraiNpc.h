#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
class EventFlowExecutor;
class MessageTagDataHolder;
class PlacementId;
}  // namespace al

class FukuwaraiFaceParts;
class FukuwaraiWatcher;
class NpcJointLookAtController;
class NpcStateReaction;
class TalkNpcCap;
class TalkNpcParam;

class FukuwaraiNpc : public al::LiveActor,
                     public al::IEventFlowEventReceiver,
                     public al::IEventFlowQueryJudge {
public:
    FukuwaraiNpc(const char* name, FukuwaraiWatcher* watcher, al::LiveActor* face, bool isMario);

    void init(const al::ActorInitInfo& info) override;
    s32 getNextLv() const;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    void control() override;
    bool receiveEvent(const al::EventFlowEventData* data) override;
    const char* judgeQuery(const char* query) const override;

    void exeWait();
    void exeReaction();
    void exePlay();
    void exeMemorize();
    void exeEventMemorize();
    void exeEventStartResult();
    void exeEventStartResultEnd();
    void exeEventResult();
    void exeEventEnd();

    bool isCompleteKuriboMario() const;
    bool isSetStartPosition() const;
    bool isMemorize() const;
    bool isPlay() const;
    bool isWaitStartResult() const;
    void startResultWipe();
    bool isEventStartResultEnd() const;
    void startResult(s32 score);
    bool isEventResultJudgeEnd() const;
    void startEndResult();
    bool isEnd() const;
    void reset();

private:
    sead::FixedSafeString<128> mEventFlowName;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    FukuwaraiWatcher* mWatcher = nullptr;
    void* _1c0 = nullptr;
    sead::Quatf mQuat = sead::Quatf::unit;
    sead::Vector3f mPosition = sead::Vector3f::zero;
    s32 mScore = 0;
    s32 mSuccessScore = 60;
    bool _1ec = false;
    bool mIsPlayedThisScene = false;
    bool mIsPlayedThisSceneLv2 = false;
    al::PlacementId* mShineActorPlacementId = nullptr;
    al::PlacementId* mMoonLockPlacementId = nullptr;
    bool mIsMarioFace = false;
    bool mIsCompleteKuriboMario = false;
    NpcStateReaction* mNpcStateReaction = nullptr;
    TalkNpcParam* mTalkNpcParam = nullptr;
    NpcJointLookAtController* mNpcJointLookAtController = nullptr;
    TalkNpcCap* mTalkNpcCap = nullptr;
};

static_assert(sizeof(FukuwaraiNpc) == 0x228);
