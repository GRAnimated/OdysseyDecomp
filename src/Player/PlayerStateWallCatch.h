#pragma once

#include <basis/seadTypes.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class CollisionParts;
class LiveActor;
}
class IUsePlayerCeilingCheck;
class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerActionCollisionSnap;
class PlayerAnimator;
class PlayerAreaChecker;
class PlayerConst;
class PlayerInput;
struct PlayerJointParamGrab;
class PlayerJudgePreInputJump;
class PlayerJudgePreInputPoleClimbSwing;
class PlayerModelHolder;
class PlayerTrigger;

class PlayerStateWallCatch : public al::ActorStateBase {
public:
    PlayerStateWallCatch(al::LiveActor* player, const PlayerConst* pConst,
                         const PlayerInput* input, IUsePlayerCollision* collision,
                         const IUsePlayerCeilingCheck* ceilingCheck,
                         const PlayerModelHolder* modelHolder,
                         const PlayerAreaChecker* areaChecker, PlayerAnimator* animator,
                         PlayerTrigger* trigger, PlayerJudgePreInputJump* judgePreInputJump,
                         PlayerJointParamGrab* grabJoint);

    void appear() override;
    bool initIgnoreFallInput();
    void kill() override;
    bool isWallCatchForm() const;
    bool update() override;
    void control() override;
    void setup(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
               const sead::Vector3f& front, const sead::Vector3f& up);
    void endFallFromWall();
    bool isClimbJump() const;
    bool isClimbJumpFall() const;
    bool isFallEnd() const;
    bool isEnableIK() const;
    bool isEnableTrample() const;
    bool isEnableDamage() const;
    sead::Vector3f getWallCatchFront() const;
    const sead::Vector3f& getCeilingCheckPos() const;
    void exeStart();
    bool followCollision(bool* isWallLost, bool allowMove);
    bool enableClimb();
    bool tryStartClimbFallMove();
    void exeWait();
    bool updateWallCatchKeep(bool isKeep);
    void exeMoveLeft();
    void initMoveFrameLeftRight();
    void exeMoveRight();
    void exeClimb();
    bool followCollisionClimb();
    void endClimb();
    void exeClimbFast();
    void exeJump();
    void exeEndFall();
    void moveCatchPos(const al::CollisionParts* collisionParts,
                      const sead::Vector3f& position, const sead::Vector3f& front,
                      const sead::Vector3f& up);
    ~PlayerStateWallCatch() override;

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    IUsePlayerCollision* mCollision;
    const IUsePlayerCeilingCheck* mCeilingCheck;
    const PlayerModelHolder* mModelHolder;
    const PlayerAreaChecker* mAreaChecker;
    PlayerAnimator* mAnimator;
    PlayerTrigger* mTrigger;
    PlayerActionAirMoveControl* mAirMoveControl;
    PlayerActionCollisionSnap* mCollisionSnap;
    PlayerJudgePreInputJump* mJudgePreInputJump;
    PlayerJointParamGrab* mGrabJoint;
    sead::Vector3f _80;
    s32 _8c;
    sead::Vector2f mWallCatchInputDir;
    sead::Vector2f mWallCatchStick;
    sead::Vector3f _a0;
    bool _ac;
    bool _ad;
    u8 _ae[2];
    PlayerJudgePreInputPoleClimbSwing* mJudgePreInputPoleClimbSwing;
};

static_assert(sizeof(PlayerStateWallCatch) == 0xB8);
