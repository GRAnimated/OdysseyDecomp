#pragma once

#include <math/seadQuat.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

class CapTargetInfo;
class NpcStateReaction;
class NpcStateReactionParam;
class NpcStateWait;
class NpcStateWaitParam;
class YukimaruStateHack;
class YukimaruStateMove;

namespace al {
class EventFlowExecutor;
}  // namespace al

class Yukimaru : public al::LiveActor, public al::IEventFlowEventReceiver {
public:
    Yukimaru(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void initAfterPlacement() override;
    void movement() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;
    bool isEnableHack() const;
    void afterHack();
    bool isEnableReaction() const;
    bool receiveEvent(const al::EventFlowEventData* event) override;
    void disableHack();
    void enableHack();

    void exeEvent();
    void exeReaction();
    void exeHack();
    void exeHackAfter();

private:
    friend class YukimaruStateHack;
    CapTargetInfo* mCapTargetInfo = nullptr;
    NpcStateWaitParam* mStateWaitParam = nullptr;
    NpcStateWait* mStateWait = nullptr;
    NpcStateReactionParam* mReactionParam = nullptr;
    NpcStateReaction* mStateReaction = nullptr;
    YukimaruStateHack* mStateHack = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    sead::Quatf mQuat = sead::Quatf::unit;
    bool mIsEnableHack = true;
    bool _159 = false;
};

static_assert(sizeof(Yukimaru) == 0x160);
