#pragma once

#include <math/seadMatrix.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class JointSpringControllerHolder;
}  // namespace al

class EnemyStateReset;
class IUsePlayerHack;
class RadiconCarStateAutoMove;

class RadiconCar : public al::LiveActor {
public:
    RadiconCar(IUsePlayerHack** hackPtr);

    void init(const al::ActorInitInfo& initInfo) override;
    void initAfterPlacement() override;
    void makeActorAlive() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;

    void setTypeRace();
    void startHackMove();
    void endHackMove();
    bool isMoving() const;
    bool isBroken() const;
    void tryCancelHack() const;
    void updateVelocity();
    void exeWait();
    void calcAccel(sead::Vector3f* accel, bool isOnGround);
    f32 calcThrottleInputValue() const;
    f32 calcSteeringInputValue() const;
    void exeMove();
    void endMoveRun();
    void exeMoveAuto();
    void endMoveAuto();
    void exeReaction();
    void exeBreak();
    void exeReset();
    void exeAppear();
    void exeStartHackMove();

private:
    void updateEffect();

    RadiconCarStateAutoMove* mStateAutoMove = nullptr;
    EnemyStateReset* mStateReset = nullptr;
    IUsePlayerHack** mPlayerHackPtr = nullptr;
    al::JointSpringControllerHolder* mJointSpringControllerHolder = nullptr;
    s32 mAirFrameCount = 0;
    sead::Matrix34f mEffectMtx = sead::Matrix34f::ident;
    bool mIsHacking = false;
    bool mIsRace = false;
};

static_assert(sizeof(RadiconCar) == 0x160);
