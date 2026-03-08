#pragma once

#include "Library/Obj/PartsModel.h"

namespace al {
struct ActorInitInfo;
class EventFlowExecutor;
class HitSensor;
class SensorMsg;
}  // namespace al

class HideAndSeekCapMan : public al::PartsModel {
public:
    HideAndSeekCapMan(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void control() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;

    void exeAppear();
    void exeWait();
    void exeDisappear();
    void exeHide();
    void exeScared();

    static HideAndSeekCapMan* tryCreateLinkNpcIfClearGame(al::LiveActor* parent,
                                                          const al::ActorInitInfo& info,
                                                          const char* suffix, const char* linkName);

private:
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    s32 mShineIdx = -1;
    s32 mCounter = 0;
};

static_assert(sizeof(HideAndSeekCapMan) == 0x158);
