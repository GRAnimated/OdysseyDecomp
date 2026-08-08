#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class CollisionParts;
class HitSensor;
class LiveActor;
}  // namespace al
class IUsePlayerCollision;
class PlayerActionCollisionSnap;
class PlayerActionDiveInWater;
class PlayerAnimator;
class PlayerConst;
class PlayerInput;
struct PlayerJointParamHandLegAngle;
class PlayerJudgePreInputJump;
class PlayerJudgePreInputPoleClimbSwing;
class PlayerModelHolder;
class PlayerStateNormalWallJump;
class PlayerTrigger;
class PlayerWallActionHistory;

class PlayerStatePoleClimb : public al::ActorStateBase {
public:
    PlayerStatePoleClimb(al::LiveActor* player, const PlayerConst* pConst, const PlayerInput* input,
                         const PlayerTrigger* trigger, PlayerModelHolder* modelHolder,
                         IUsePlayerCollision* collision, PlayerAnimator* animator,
                         PlayerWallActionHistory* wallActionHistory,
                         PlayerJointParamHandLegAngle* handLegAngle,
                         PlayerJudgePreInputJump* judgePreInputJump,
                         PlayerActionDiveInWater* actionDiveInWater);
    ~PlayerStatePoleClimb() override;

    void appear() override;
    void kill() override;
    bool isFormPoleClimb() const;
    bool update() override;
    void setup(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
               const sead::Vector3f& front, const sead::Vector3f& up, f32 depth, f32 moveRate,
               const char* animationName);
    void updatePoleDepth(f32 depth, f32 moveRate);
    bool isAttachPole() const;
    bool isPoleJump() const;
    bool isTriggerReaction() const;
    bool isEnableTrample() const;
    bool isForceFollowCap() const;
    const sead::Vector3f& getPoleFront() const;
    al::HitSensor* getPoleSensor() const;

    const char* getMaterialCode() const { return mMaterialCode; }

    void updateLeavePoleTrans() const;
    void exeStart();
    bool followCollision();
    bool tryStartClimbMove(s32 direction, s32* moveDirection);
    void exeWait();
    void exeUp();
    void changeUpMoveSpeed(s32 startFrame, s32 endFrame);
    void exeDown();
    void exeTurn();
    void exeJump();
    void exeTopStart();
    void exeTopWait();
    bool tryTurnTopOrClimb();
    void exeTopTurn();
    void exeTopEnd();
    void exeTopJump();

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    PlayerModelHolder* mModelHolder;
    IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerJointParamHandLegAngle* mHandLegAngle;
    PlayerJudgePreInputJump* mJudgePreInputJump;
    PlayerStateNormalWallJump* mWallJump;
    PlayerStateNormalWallJump* mTopJump;
    PlayerWallActionHistory* mWallActionHistory;
    PlayerActionCollisionSnap* mCollisionSnap;
    PlayerJudgePreInputPoleClimbSwing* mJudgePreInputPoleClimbSwing;
    const char* mMaterialCode;
    f32 _88;
    f32 _8c;
    f32 _90;
    s32 _94;
    s32 _98;
    sead::Vector2f _9c;
    bool _a4;
    bool _a5;
    u8 _a6[2];
    sead::Vector2f _a8;
    s32 _b0;
    bool _b4;
    u8 _b5[3];
    sead::Vector2f _b8;
};

static_assert(sizeof(PlayerStatePoleClimb) == 0xC0);
