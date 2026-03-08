#pragma once

#include <prim/seadSafeString.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

class FukuwaraiWatcher;

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
    void* _1b0 = nullptr;
    al::LiveActor* mFace = nullptr;
    void* _1c0 = nullptr;
    sead::Quatf mQuat = sead::Quatf::unit;
    sead::Vector3f mPosition = sead::Vector3f::zero;
    s32 _1e4 = 0;
    s32 _1e8 = 60;
    s16 _1ec = 0;
    bool _1ee = false;
    u8 _1ef = 0;
    void* _1f0 = nullptr;
    void* _1f8 = nullptr;
    bool mIsMarioFace = false;
    bool _201 = false;
    u8 _202[6] = {};
    void* _208 = nullptr;
    void* _210 = nullptr;
    void* _218 = nullptr;
    void* _220 = nullptr;
};

static_assert(sizeof(FukuwaraiNpc) == 0x228);
