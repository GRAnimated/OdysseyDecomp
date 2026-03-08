#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

struct ActorInitInfo;

namespace al {
class EventFlowExecutor;
class HitSensor;
class SensorMsg;
}  // namespace al

class JumpingRopeLayout;
class Shine;
struct ActionAnimInfo;

struct RawPtrArray {
    s32 ptrNum = 0;
    s32 ptrNumMax = 0;
    void** ptrs = nullptr;
};

class JumpingRopeNpc : public al::LiveActor {
public:
    JumpingRopeNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    void control() override;
    void movement() override;

    void exeStart();
    void exeWait();
    void exeJump();
    void endJump();
    void exeMiss();
    void exeInterrupt();
    void exeTryAgain();

    bool tryMiss();
    void updateAnim(bool sync);
    void updateRot(bool updateFrame);
    void updateRope();
    bool checkRopeCollision() const;
    void miss();
    bool isActionEnd() const;
    void restoreInterval();
    bool isEnableDisplayBalloon() const;
    void interrupt();
    s32 searchNearestNode();
    void startResultAction();
    void startResultMessage();
    bool tryStartResultLoopAction();
    bool isPlayerOff() const;
    void startReactionPartner();
    void calcRopePos(sead::Vector3f* out, s32 index, f32 t) const;
    void calcAndSetJointMtx(sead::Matrix34f* mtx, s32 jointIdx);

    s32 getBest() const;
    void setBest(s32 best);
    void setBestToday(s32 best);
    s32 getBestToday() const;

    bool isNerveJump() const;
    bool isNerveMiss() const;

private:
    sead::Vector3f mRopeCenter = sead::Vector3f::zero;
    f32 mHalfRopeLength = 0.0f;
    sead::Vector3f mHandTarget = sead::Vector3f::zero;
    sead::Vector3f mPartnerHandTarget = sead::Vector3f::zero;
    al::LiveActor* mPartner = nullptr;
    al::LiveActor* mRopeJointController = nullptr;
    RawPtrArray mRopeNodes;
    sead::Vector3f mSensorPos = sead::Vector3f::zero;
    u32 _15c = 0;
    RawPtrArray mCollisionEntries;
    void* mCollisionFreeHead = nullptr;
    void* mCollisionFreeBase = nullptr;
    u16 _180 = 0;
    bool _182 = false;
    al::LiveActor* mRopeActor = nullptr;
    s32 mJointCount = 0;
    RawPtrArray mJointControllers;
    void* mJointDataAlloc = nullptr;
    void* mJointDataFreeHead = nullptr;
    f32 mNodeInterval = 0.0f;
    f32 mAngle = 0.0f;
    f32 mRopeSpeed = 5.0f;
    f32 mIntervalMul = 0.95f;
    Shine* mShine1 = nullptr;
    Shine* mShine2 = nullptr;
    bool _1d8 = false;
    bool _1d9 = false;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    al::EventFlowExecutor* mPartnerEventFlowExecutor = nullptr;
    JumpingRopeLayout* mLayout = nullptr;
    s32 _1f8 = 0;
    bool _1fc = true;
    s32 _200 = 30;
    s32 _204 = 100;
    s32 mJumpCount = 0;
    bool _20c = true;
    bool _20d = false;
    bool _20e = false;
    s32 mInvincibilityTimer = 0;
    ActionAnimInfo* mActionAnimInfo = nullptr;
    ActionAnimInfo* mPartnerActionAnimInfo = nullptr;
};

static_assert(sizeof(JumpingRopeNpc) == 0x228);
