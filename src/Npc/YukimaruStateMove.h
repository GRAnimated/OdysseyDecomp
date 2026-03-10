#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class AnimScaleController;
class HitSensor;
class LiveActor;
class SensorMsg;
}  // namespace al

namespace sead {
template <typename T>
struct Quat;
using Quatf = Quat<f32>;
}  // namespace sead

class YukimaruInput;

class YukimaruMovement {
public:
    static bool attackSensor(al::LiveActor* actor, al::HitSensor* self, al::HitSensor* other);
    static void updateVelocity(al::LiveActor* actor);
};

class YukimaruStateMove : public al::ActorStateBase, public YukimaruMovement {
public:
    YukimaruStateMove(al::LiveActor* actor, YukimaruInput* input, sead::Quatf* quat);

    void attackSensor(al::HitSensor* self, al::HitSensor* other);
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other);

    void startRun(bool isRunning);
    void resetInput();

    void exeRunStart();
    void updateMove();
    void exeRun();
    void exeJump();
    void exeJumpEnd();

    void updateMoveNoInput();
    void updateMove(YukimaruInput* input);
    void reactionBound(f32 speed, bool isUpward);
    void updateScale();
    void startScaleBoundReaction(f32 speed);
    void resetEffect();

private:
    YukimaruInput* mInput;
    sead::Quatf* mQuat;
    sead::Matrix34f mEffectMtx;
    sead::Vector3f mVelocity;
    sead::Vector3f mCollidedNormal;
    sead::Vector3f mRotation;
    s32 mInputTimer;
    s32 mCollisionBounceTimer;
    s32 mBounceCount;
    s32 mSlideTimer;
    s32 mSandSinkTimer;
    s32 mGravityTimer;
    f32 mSpeed;
    s32 mJumpType;
    bool mIsJumping;
    bool mIsHack;
    al::AnimScaleController* mScaleController;
    al::AnimScaleController* mBoundScaleController;

    friend class YukimaruStateHack;
};

static_assert(sizeof(YukimaruStateMove) == 0xB8);
