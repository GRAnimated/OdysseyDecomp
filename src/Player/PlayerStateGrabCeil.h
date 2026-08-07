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
    PlayerStateGrabCeil(al::LiveActor*, const PlayerConst*, const PlayerInput*,
                        const PlayerModelHolder*, PlayerTrigger*, IUsePlayerCollision*,
                        PlayerAnimator*, PlayerJudgePreInputJump*, al::HitSensor*,
                        PlayerJointParamGrab*);
    ~PlayerStateGrabCeil() override;

    void appear() override;
    void followCollision();
    void sendMsgStartGrab();
    void kill() override;
    void sendMsgEndGrab();
    void setup(const al::CollisionParts*, const sead::Vector3f&, const sead::Vector3f&,
               const sead::Vector3f&);
    bool isFormGrabCeil() const;
    bool isJump() const;
    bool isEnableNextGrabCeil() const;
    bool isEnableSnapForce() const;
    bool isEnableTrample() const;
    const sead::Vector3f& getGrabCeilFront() const;
    void exeStart();
    void leaveGrabCeil(bool);
    void updateWaitSwingFlag(bool, f32);
    void exeWait();
    void exeSwing();
    void exeJump();
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
    PlayerActionAirMoveControl* mAirMoveControl;
    PlayerActionCollisionSnap* mCollisionSnap;
    bool _78;
    bool _79;
    u8 _7a[2];
    s32 _7c;
    bool _80;
    u8 _81[3];
    f32 _84;
    f32 _88;
    f32 _8c;
    f32 _90;
    sead::Vector3f _94;
    sead::Vector3f _a0;
    s32 _ac;
    f32 _b0;
    s32 _b4;
    f32 _b8;
    f32 _bc;
    f32 _c0;
    u8 _c4[4];
};

static_assert(sizeof(PlayerStateGrabCeil) == 0xC8);
