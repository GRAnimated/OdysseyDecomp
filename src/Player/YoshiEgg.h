#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
}

class FukankunZoomCapMessage;
class IUsePlayerCollision;

class YoshiEgg : public al::LiveActor {
public:
    YoshiEgg(const al::LiveActor* host, const IUsePlayerCollision* collision);

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    void initPlacementEgg();
    void appearEgg();
    bool isEndAppear() const;
    bool isBreak() const;
    void exeAppear();
    void exeWait();
    void exeBreak();
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;

private:
    const al::LiveActor* mHost;
    const IUsePlayerCollision* mCollision;
    FukankunZoomCapMessage* mZoomCapMessage = nullptr;
};

static_assert(sizeof(YoshiEgg) == 0x120);
