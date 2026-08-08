#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Player/HackerStateBase.h"

namespace al {
class CollisionParts;
class MtxConnector;
class LiveActor;
}  // namespace al

class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerCounterForceRun;
class PlayerEffect;
class YoshiJudgeFallFromGround;
class YoshiActionTongueAttack;
class YoshiStateHackRun;
class YoshiTongue;

class YoshiStateHackTongueShrink : public HackerStateBase {
public:
    YoshiStateHackTongueShrink(al::LiveActor* actor, IUsePlayerHack** playerHack,
                               al::LiveActor* modelActor, const PlayerConst* playerConst,
                               const YoshiTongue* tongue,
                               const PlayerCounterForceRun* counterForceRun,
                               YoshiActionTongueAttack* actionTongueAttack,
                               IUsePlayerCollision* collision, PlayerAnimator* animator,
                               PlayerEffect* effect);
    void appear() override;
    void kill() override;

    void setupGroundSnap(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
                         const sead::Vector3f& front, const sead::Vector3f& up);
    void setupWallSnap(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
                       const sead::Vector3f& normal, const sead::Vector3f& up,
                       bool isCollisionShapeJump);
    bool isEnableAccelForceRun() const;
    bool isEnablePullForce() const;
    bool isEnableWallCling() const;
    bool isEnableShrinkEndJump() const;
    bool isEnableTongueKeepAction() const;
    bool isJumpRolling() const;
    bool isCollisionShapeJump() const;
    bool isGroundConnectRun() const;
    bool isEndCancelForceRun() const;
    s32 getLoopRunCount() const;
    void endShrinkAndJump();
    void endLoopRunMoveDirection();
    void exeRun();
    void exeLoopRun();
    void exeJump();
    void exeJumpKeep();
    void exeFall();
    ~YoshiStateHackTongueShrink() override;

private:
    const PlayerConst* mPlayerConst;
    const YoshiTongue* mTongue;
    YoshiActionTongueAttack* mActionTongueAttack;
    IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    YoshiJudgeFallFromGround* mJudgeFallFromGround;
    YoshiStateHackRun* mStateRun;
    PlayerActionAirMoveControl* mAirMoveControl;
    const al::CollisionParts* mCollisionParts;
    al::MtxConnector* mMtxConnector;
    sead::Matrix34f mSnapMtx;
    void* _a8;
    u8 _b0[0x4c];
    bool _fc;
    bool _fd;
    u8 _fe[2];
    u32 mLoopRunCount;
    u32 _104;
    u64 _108;
    u64 _110;
    s32 _118;
    s32 _11c;
};

static_assert(sizeof(YoshiStateHackTongueShrink) == 0x120);
