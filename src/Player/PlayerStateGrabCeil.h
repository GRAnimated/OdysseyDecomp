#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class CollisionParts;
class HitSensor;
class LiveActor;
}

class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerActionCollisionSnap;
class PlayerAnimator;
class PlayerConst;
class PlayerInput;
struct PlayerJointParamGrab;
class PlayerJudgePreInputJump;
class PlayerModelHolder;
class PlayerTrigger;

class PlayerStateGrabCeil : public al::ActorStateBase {
public:
    PlayerStateGrabCeil(al::LiveActor* actor, const PlayerConst* playerConst,
                        const PlayerInput* input, const PlayerModelHolder* modelHolder,
                        PlayerTrigger* trigger, IUsePlayerCollision* collision,
                        PlayerAnimator* animator, PlayerJudgePreInputJump* judgePreInputJump,
                        al::HitSensor* sensor, PlayerJointParamGrab* jointParamGrab);
    void appear() override;
    bool followCollision();
    void sendMsgStartGrab();
    void kill() override;
    void sendMsgEndGrab();
    void setup(const al::CollisionParts* parts, const sead::Vector3f& position,
               const sead::Vector3f& front, const sead::Vector3f& up);
    bool isFormGrabCeil() const;
    bool isJump() const;
    bool isEnableNextGrabCeil() const;
    bool isEnableSnapForce() const;
    bool isEnableTrample() const;
    const sead::Vector3f& getGrabCeilFront() const;
    void exeStart();
    void leaveGrabCeil(bool isJumpInput);
    void updateWaitSwingFlag(bool isSwingInput, f32 energy);
    void exeWait();
    void exeSwing();
    void exeJump();
    ~PlayerStateGrabCeil() override;
private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const PlayerModelHolder* mModelHolder;
    PlayerTrigger* mTrigger;
    IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerJudgePreInputJump* mJudgePreInputJump;
    al::HitSensor* mSensor;
    PlayerJointParamGrab* mJointParamGrab;
    PlayerActionAirMoveControl* mAirMoveControl = nullptr;
    PlayerActionCollisionSnap* mCollisionSnap = nullptr;
    bool _78 = false;
    bool _79 = false;
    u8 _7a[2];
    s32 _7c = 0;
    bool _80 = false;
    u8 _81[3];
    f32 _84 = 0.0f;
    f32 _88 = 0.0f;
    f32 _8c = 0.0f;
    f32 _90 = 0.0f;
    sead::Vector3f _94 = {0.0f, 0.0f, 0.0f};
    sead::Vector3f _a0 = {0.0f, 0.0f, 0.0f};
    s32 _ac = 0;
    f32 _b0 = 0.0f;
    s32 _b4 = 0;
    f32 _b8 = 0.0f;
    s32 _bc = 0;
    f32 _c0 = 0.0f;
    u8 _c4[4];
};

static_assert(sizeof(PlayerStateGrabCeil) == 0xC8);
