#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/HackerStateBase.h"

namespace al {
struct ActorInitInfo;
class HitSensor;
class LiveActor;
class SensorMsg;
}

class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerAnimator;
class PlayerConst;
class PlayerEffect;
class PlayerCounterForceRun;
class PlayerEyeSensorHitHolder;
class PlayerWallActionHistory;
class HackerJudgeStartJump;
class HackerJudgeStartRun;
class YoshiJudgeFallFromGround;
class YoshiJudgeStartJumpFlap;
class YoshiJudgeStartTongueShrink;
class YoshiJudgeWallCling;
class YoshiJudgeWallHitDown;
class YoshiStateHackDown;
class YoshiStateHackFall;
class YoshiStateHackJump;
class YoshiStateHackJumpFlap;
class YoshiStateHackRun;
class YoshiStateHackTongueShrink;
class YoshiStateHackWait;
class YoshiStateHackWallAir;
class YoshiStateHackWallPopUp;
class YoshiActionTongueAttack;
struct PlayerJumpMessageRequest;
class PlayerTrigger;
class YoshiTongue;

class YoshiStateHackPlay : public HackerStateBase {
public:
    YoshiStateHackPlay(const al::ActorInitInfo& info, al::LiveActor* actor, al::LiveActor* modelActor,
                       IUsePlayerHack** playerHack, const PlayerConst* playerConst,
                       PlayerTrigger* trigger, PlayerAnimator* animator,
                       IUsePlayerCollision* collision, PlayerEffect* effect,
                       PlayerJumpMessageRequest* jumpMessageRequest);

    void appear() override;
    void setNerveOnGround();
    void kill() override;
    bool update() override;
    void executePreUpdateNerveChange();
    void updatePrevMovement();
    void updateAfterMovement();
    void prepareEndHack();
    bool reactionCollidedCollisionCodeDamage() const;
    bool isActiveHeadCorrection() const;
    bool isCollisionShapeTongueJump() const;
    bool tryGetLookAtTonguePos(sead::Vector3f* position) const;
    bool tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const;
    void calcGroundPoseRate(f32* frontRate, f32* sideRate) const;
    void startFruitShineGetDemo();
    void exeWait();
    bool tryChangeNerveOnGround();
    void exeJump();
    bool tryChangeNerveAir();
    void exeFlap();
    void exeWallAir();
    void setNerveTongueShrink();
    void exeWallPopUp();
    void exeTongueShrink();
    void exeFall();
    void exeRun();
    void exeDown();
    bool attackSensor(al::HitSensor* self, al::HitSensor* other);
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self);
    ~YoshiStateHackPlay() override;

private:
    al::LiveActor* _28;
    const PlayerConst* mConst;
    PlayerTrigger* mTrigger;
    PlayerAnimator* mAnimator;
    IUsePlayerCollision* mCollision;
    PlayerJumpMessageRequest* mJumpMessageRequest;
    YoshiTongue* mTongue;
    YoshiActionTongueAttack* mTongueAttack;
    PlayerWallActionHistory* mWallActionHistory;
    PlayerEyeSensorHitHolder* mEyeSensorHitHolder;
    PlayerCounterForceRun* mCounterForceRun;
    YoshiStateHackWait* mStateWait;
    YoshiStateHackRun* mStateRun;
    YoshiStateHackJump* mStateJump;
    YoshiStateHackJumpFlap* mStateJumpFlap;
    YoshiStateHackWallAir* mStateWallAir;
    YoshiStateHackWallPopUp* mStateWallPopUp;
    YoshiStateHackTongueShrink* mStateTongueShrink;
    YoshiStateHackFall* mStateFall;
    YoshiStateHackDown* mStateDown;
    HackerJudgeStartJump* mJudgeStartJump;
    HackerJudgeStartRun* mJudgeStartRun;
    YoshiJudgeFallFromGround* mJudgeFallFromGround;
    YoshiJudgeStartJumpFlap* mJudgeStartJumpFlap;
    YoshiJudgeStartTongueShrink* mJudgeStartTongueShrink;
    YoshiJudgeWallCling* mJudgeWallCling;
    YoshiJudgeWallHitDown* mJudgeWallHitDown;
};

static_assert(sizeof(YoshiStateHackPlay) == 0x100);
