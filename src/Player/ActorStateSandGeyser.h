#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class LiveActor;
class SensorMsg;
}

class ActorStateSandGeyser : public al::ActorStateBase {
public:
    ActorStateSandGeyser(al::LiveActor* actor);
    ~ActorStateSandGeyser() override;

    void appear() override;
    void kill() override;
    void tryEndPadRumble();
    void exeSandGeyser();
    void tryStartPadRumble();
    bool receiveMsgSandGeyser(const al::SensorMsg* message, const al::HitSensor* sensor);
    bool isRequested() const { return mRequestSensor; }
    void clearRequest() { mRequestSensor = nullptr; }

private:
    al::HitSensor* mRequestSensor = nullptr;
    f32 mTargetHeight = 0.0f;
    f32 mMoveSpeed = 0.0f;
    bool mIsPadRumbleStarted = false;
    u8 mPadding31[0x38 - 0x31];
};

static_assert(sizeof(ActorStateSandGeyser) == 0x38);
