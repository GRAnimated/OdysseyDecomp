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
    bool tryGetLookAtPlayerPos(sead::Vector3f* position) const;
    bool reactionCollidedCollisionCode();
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self);
    bool receiveMsgReturnEggAndInitPosition(const al::SensorMsg* message, al::HitSensor* other,
                                            al::HitSensor* self);
    ~YoshiStateNpc() override;

private:
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    const PlayerTrigger* mTrigger;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    al::EventFlowExecutor* mEventFlowExecutor;
    PlayerAnimator* mAnimator;
    sead::Vector3f mTurnUp = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTurnStartFront = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTurnTargetFront = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mLookAtPlayerOffset = {0.0f, 0.0f, 0.0f};
    bool mIsHackEnd = false;
    bool mIsUseReturnTimer = false;
    u8 _82[2];
    s32 mReturnTimer = 0;
};

static_assert(sizeof(YoshiStateNpc) == 0x88);
