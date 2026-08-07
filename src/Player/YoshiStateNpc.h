#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class EventFlowExecutor;
class HitSensor;
class LiveActor;
class SensorMsg;
class WaterSurfaceFinder;
}

class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;
class PlayerTrigger;

class YoshiStateNpc : public al::ActorStateBase {
public:
    YoshiStateNpc(al::LiveActor* actor, const PlayerConst* playerConst,
                  const IUsePlayerCollision* collision, const PlayerTrigger* trigger,
                  const al::WaterSurfaceFinder* waterSurfaceFinder,
                  const al::ActorInitInfo& info, PlayerAnimator* animator,
                  al::EventFlowExecutor* eventFlowExecutor);

    void appear() override;
    void control() override;
    void exeAppear();
    void exeWait();
    void exeTurn();
    void exeReaction();
    void exeHackEnd();
    bool reactionCollidedCollisionCode();
    bool tryGetLookAtPlayerPos(sead::Vector3f* position) const;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self);
    bool receiveMsgReturnEggAndInitPosition(const al::SensorMsg* message, al::HitSensor* other,
                                            al::HitSensor* self);

private:
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    const PlayerTrigger* mTrigger;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    al::EventFlowExecutor* mEventFlowExecutor;
    PlayerAnimator* mAnimator;
    void* _50 = nullptr;
    void* _58 = nullptr;
    void* _60 = nullptr;
    void* _68 = nullptr;
    void* _70 = nullptr;
    void* _78 = nullptr;
    bool mIsHackEnd = false;
    bool mIsUseReturnTimer = false;
    u8 _82[2]{};
    s32 mReturnTimer = 0;
};

static_assert(sizeof(YoshiStateNpc) == 0x88);
