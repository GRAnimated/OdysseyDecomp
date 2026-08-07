#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/IUsePlayerCeilingCheck.h"
#include "Player/IUsePlayerCollision.h"
#include "Player/IUsePlayerHeightCheck.h"

namespace al {
struct ActorInitInfo;
class EventFlowExecutor;
class HitSensor;
class LiveActor;
class SensorMsg;
class WaterSurfaceFinder;
}  // namespace al

class CapTargetInfo;
class PlayerAnimator;
class PlayerConst;
class PlayerEffect;
struct PlayerJumpMessageRequest;
class PlayerModelHolder;
class PlayerTrigger;
class PlayerCollider;

class PlayerColliderYoshi : public IUsePlayerCollision,
                            public IUsePlayerCeilingCheck,
                            public IUsePlayerHeightCheck {
public:
    PlayerColliderYoshi(al::LiveActor* actor);

    void init();
    void tryChangeCollisionNormal();
    void updateCollider(const sead::Vector3f& velocity);
    void updateHeightCheck(const sead::Vector3f& trans, const sead::Vector3f& up);
    void updateCeilingCheck(const sead::Vector3f& trans, const sead::Vector3f& up, f32 offset);
    void tryChangeCollisionTongueJump();

    PlayerCollider* getPlayerCollider() const override;
    bool isEnableStandUp() const override;
    bool isEnableHoldUp() const override;
    bool isPressedCeil() const override;
    f32 getSafetyCeilSpace() const override;
    f32 getCeilCheckHeight() const override;
    bool isAboveGround() const override;
    f32 getGroundHeight() const override;
    f32 getShadowDropHeight() const override;

private:
    u8 _pad[0x58 - sizeof(IUsePlayerCollision) - sizeof(IUsePlayerCeilingCheck) -
            sizeof(IUsePlayerHeightCheck)];
};
static_assert(sizeof(PlayerColliderYoshi) == 0x58);

class PlayerModelChangerYoshi : public IPlayerModelChanger {
public:
    PlayerModelChangerYoshi(const al::LiveActor* actor, PlayerModelHolder* modelHolder);

    void syncHost();
    void syncModelFlag(al::LiveActor* actor);
    void appearModel();
    void killModel();
    void changeModel(al::LiveActor* actor);

    void resetPosition() override;
    void hideModel() override;
    void hideSilhouette() override;
    void hideShadowMask() override;
    void showModel() override;
    void showSilhouette() override;
    void showShadowMask() override;
    bool isFireFlower() const override;
    bool isMini() const override;
    bool isChange() const override;
    bool is2DModel() const override;
    bool isHiddenModel() const override;
    bool isHiddenShadowMask() const override;

private:
    u8 _pad[0x28 - sizeof(IPlayerModelChanger)];
};
static_assert(sizeof(PlayerModelChangerYoshi) == 0x28);

class YoshiStateEgg : public al::NerveStateBase {
public:
    YoshiStateEgg(const al::ActorInitInfo& info, al::LiveActor* actor,
                  const IUsePlayerCollision* collision, const PlayerConst* playerConst,
                  const al::WaterSurfaceFinder* waterSurfaceFinder,
                  PlayerModelChangerYoshi* modelChanger);

    bool reactionCollidedCollisionCode();

private:
    u8 _pad[0x50 - sizeof(al::NerveStateBase)];
};
static_assert(sizeof(YoshiStateEgg) == 0x50);

class YoshiStateNpc : public al::NerveStateBase {
public:
    YoshiStateNpc(al::LiveActor* actor, const PlayerConst* playerConst,
                  const IUsePlayerCollision* collision, const PlayerTrigger* trigger,
                  const al::WaterSurfaceFinder* waterSurfaceFinder,
                  const al::ActorInitInfo& info, PlayerAnimator* animator,
                  al::EventFlowExecutor* eventFlowExecutor);

    bool tryGetLookAtPlayerPos(sead::Vector3f* pos) const;
    bool reactionCollidedCollisionCode();
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self);
    bool receiveMsgReturnEggAndInitPosition(const al::SensorMsg* message, al::HitSensor* other,
                                            al::HitSensor* self);

private:
    u8 _pad[0x88 - sizeof(al::NerveStateBase)];
};
static_assert(sizeof(YoshiStateNpc) == 0x88);

class YoshiStateHack : public al::NerveStateBase {
public:
    YoshiStateHack(const al::ActorInitInfo& info, al::LiveActor* actor, al::LiveActor* modelActor,
                   const CapTargetInfo* capTargetInfo, const PlayerConst* playerConst,
                   const al::WaterSurfaceFinder* waterSurfaceFinder, PlayerTrigger* trigger,
                   PlayerModelChangerYoshi* modelChanger, PlayerAnimator* animator,
                   IUsePlayerCollision* collision, PlayerEffect* effect,
                   PlayerJumpMessageRequest* jumpMessageRequest);

    void updatePrevMovement();
    void updateAfterMovement();
    bool isEnableUpdateCollider() const;
    bool isActiveHeadCorrection() const;
    bool isCollisionShapeTongueJump() const;
    bool tryGetLookAtTonguePos(sead::Vector3f* pos) const;
    bool tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const;
    bool calcGroundPoseRate(f32* poseRate, f32* poseRateSide) const;
    void startFruitShineGetDemo();
    bool attackSensor(al::HitSensor* self, al::HitSensor* other);
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self);
    bool receiveMsgSetNerveState(const al::SensorMsg* message, al::HitSensor* other,
                                 al::HitSensor* self);
    bool receiveMsgEndState(bool* isReturnEgg, const al::SensorMsg* message,
                            al::HitSensor* other, al::HitSensor* self);
    bool reactionCollidedCollisionCode();

private:
    u8 _pad[0x78 - sizeof(al::NerveStateBase)];
};
static_assert(sizeof(YoshiStateHack) == 0x78);

class YoshiJointControlKeeper {
public:
    YoshiJointControlKeeper(const al::LiveActor* modelActor,
                            const IUsePlayerCollision* collision);
    void update();
    void setLookAtActive(bool active) { mIsLookAtActive = active; }

    const al::LiveActor* mModelActor;
    const IUsePlayerCollision* mCollision;
    void* mHeadCorrection;
    void* mLookAtController;
    void* mGroundPose;
    bool mIsHeadCorrectionActive;
    u8 _29[3];
    f32 mHeadCorrectionRate;
    u8 mIsLookAtActive;
    u8 _31[3];
    sead::Vector3f mLookAtPosition;
    sead::Vector2f mGroundPoseRates;
    f32 mGroundPoseRateCurrent;
    f32 mGroundPoseRateSideCurrent;
};

static_assert(sizeof(YoshiJointControlKeeper) == 0x50);
