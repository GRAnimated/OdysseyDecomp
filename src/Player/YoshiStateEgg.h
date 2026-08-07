#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class LiveActor;
class WaterSurfaceFinder;
}

class IUsePlayerCollision;
class PlayerConst;
class PlayerModelChangerYoshi;
class YoshiEgg;

class YoshiStateEgg : public al::ActorStateBase {
public:
    YoshiStateEgg(const al::ActorInitInfo& info, al::LiveActor* actor,
                  const IUsePlayerCollision* collision, const PlayerConst* playerConst,
                  const al::WaterSurfaceFinder* waterSurfaceFinder,
                  PlayerModelChangerYoshi* modelChanger);

    void appear() override;
    void kill() override;
    void exeAppear();
    void exeWait();
    bool reactionCollidedCollisionCode();

private:
    const IUsePlayerCollision* mCollision;
    const PlayerConst* mPlayerConst;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    YoshiEgg* mEgg = nullptr;
    PlayerModelChangerYoshi* mModelChanger;
    bool mIsFirstAppear = true;
};

static_assert(sizeof(YoshiStateEgg) == 0x50);
