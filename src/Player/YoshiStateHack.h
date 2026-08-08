#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class HitSensor;
class LiveActor;
class SensorMsg;
class WaterSurfaceFinder;
}

class CapTargetInfo;
class HackerDepthShadowMapCtrl;
class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerAnimator;
class PlayerConst;
class PlayerEffect;
class PlayerHackStartShaderCtrl;
struct PlayerJumpMessageRequest;
class PlayerModelChangerYoshi;
class PlayerTrigger;
class YoshiStateHackPlay;

class YoshiStateHack : public al::ActorStateBase {
public:
    YoshiStateHack(const al::ActorInitInfo& info, al::LiveActor* actor,
                   al::LiveActor* modelActor, const CapTargetInfo* capTargetInfo,
                   const PlayerConst* playerConst,
                   const al::WaterSurfaceFinder* waterSurfaceFinder, PlayerTrigger* trigger,
                   PlayerModelChangerYoshi* modelChanger, PlayerAnimator* animator,
                   IUsePlayerCollision* collision, PlayerEffect* effect,
                   PlayerJumpMessageRequest* jumpMessageRequest);

    void appear() override;
    void updatePrevMovement();
    void updateAfterMovement();
    bool isEnableUpdateCollider() const;
    bool isActiveHeadCorrection() const;
    bool isCollisionShapeTongueJump() const;
    bool tryGetLookAtTonguePos(sead::Vector3f* position) const;
    bool tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const;
    void calcGroundPoseRate(f32* frontRate, f32* sideRate) const;
    void startFruitShineGetDemo();
    void exeLockOn();
    void exeDemo();
    void exeDemoHackStart();
    void exeHack();
    bool attackSensor(al::HitSensor* self, al::HitSensor* other);
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self);
    bool receiveMsgSetNerveState(const al::SensorMsg* message, al::HitSensor* other,
                                 al::HitSensor* self);
    bool receiveMsgEndState(bool* isReturnEgg, const al::SensorMsg* message,
                            al::HitSensor* other, al::HitSensor* self);
    void prepareEndHack();
    bool reactionCollidedCollisionCode();

private:
    al::LiveActor* mModelActor;
    const PlayerConst* mPlayerConst;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    IUsePlayerCollision* mCollision;
    const CapTargetInfo* mCapTargetInfo;
    PlayerModelChangerYoshi* mModelChanger;
    PlayerAnimator* mAnimator;
    IUsePlayerHack* mPlayerHack = nullptr;
    YoshiStateHackPlay* mStateHackPlay = nullptr;
    PlayerHackStartShaderCtrl* mHackStartShaderCtrl = nullptr;
    HackerDepthShadowMapCtrl* mDepthShadowMapCtrl = nullptr;
};

static_assert(sizeof(YoshiStateHack) == 0x78);
