#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
class ActorCameraTarget;
class CollisionPartsConnector;
class EventFlowExecutor;
class HitSensor;
class JointLookAtController;
struct ActorInitInfo;
}  // namespace al

class BindKeepDemoInfo;
class IUsePlayerPuppet;
class NpcStateReaction;
class SphinxRideAim;
class SphinxRideStateRevival;
class SphinxRideStateStop;
class TalkNpcParam;

class SphinxRide : public al::LiveActor,
                   public al::IEventFlowEventReceiver,
                   public al::IEventFlowQueryJudge {
public:
    SphinxRide(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void initAfterPlacement() override;
    void movement() override;
    void control() override;
    void updateCollider() override;
    void calcAnim() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) override;
    void startClipped() override;

    bool receiveEvent(const al::EventFlowEventData* event) override;
    const char* judgeQuery(const char* query) const override;

    void updateJointPose();
    void requestGetOffForce();
    void disappear();
    bool isCanRidePlacement();
    void exeWait();
    void exeStandby();
    void setNerveFall();
    void trySlipOnMoveLimit();
    void exeReaction();
    void exeDemoStandbyStart();
    void exeDemoStandbyTurnZero();
    void exeDemoTurnEnd();
    void exeGetOnStart();
    void exeGetOnStartOn();
    void exeGetOn();
    bool isValidateCameraAngleV(f32 angle);
    void exeRun();
    bool tryGetOffAndCancelBind();
    void updateRun(f32 turnRate);
    void exeClash();
    void updateGravityAndDump(f32 hScale, f32 vScale);
    void exeStop();
    bool sendMsgCollidedCactus();
    void exeFall();
    void controlAndRotateYAndMove(f32 turnRate, f32 speed, bool adjustSlope);
    void exeLand();
    void exeGetOff();
    void exeJump();
    void exeRevival();
    void exeEventWait();
    bool isRidePlayer() const;
    bool isPlayerInputSwingSphinxStop() const;
    bool sendMsgCollidedTouch();
    bool calcCheckCollidedWallCommon(sead::Vector3f* wallPos, sead::Vector3f* wallNormal,
                                     al::HitSensor** bodySensor, al::HitSensor** wallSensor);
    void startStandby(const sead::Vector3f& trans, const sead::Vector3f& front);
    bool isNerveStandby() const;
    void setupEventFade();
    void startEventWait();
    void endEventWait(f32 startDegree);

private:
    friend class SphinxRideStateStop;

    IUsePlayerPuppet* mPlayerPuppet = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    s32 mBgmLineChangeCount = 0;
    sead::Vector3f mPrevTrans = sead::Vector3f::zero;
    bool _138 = false;
    al::ActorCameraTarget* mCameraTarget = nullptr;
    f32 mAllRootRotateZ = 0.0f;
    f32 mAllRootRotateZTarget = 0.0f;
    f32 mPlayerRotateZ = 0.0f;
    f32 mPlayerRotateZTarget = 0.0f;
    f32 mCameraAngle = -1.0f;
    f32 mStartDegree = 0.0f;
    sead::Vector3f _160 = sead::Vector3f::zero;
    sead::Vector3f mEffectFollowPos = sead::Vector3f::zero;
    al::HitSensor* mLastAttackSensor = nullptr;
    sead::Quatf mSavedPuppetQuat = sead::Quatf::unit;
    sead::Vector3f mSavedPuppetTrans = sead::Vector3f::zero;
    SphinxRideStateStop* mStateStop = nullptr;
    SphinxRideStateRevival* mStateRevival = nullptr;
    NpcStateReaction* mStateReaction = nullptr;
    bool mIsCanRidePlacement = false;
    f32 _1bc = 0.0f;
    f32 _1c0 = 0.0f;
    sead::Quatf mJointQuat = sead::Quatf::unit;
    sead::Vector3f mFrontDir = sead::Vector3f::ez;
    sead::Vector3f mUpDir = sead::Vector3f::ey;
    BindKeepDemoInfo* mBindKeepDemoInfo = nullptr;
    al::JointLookAtController* mJointLookAtController = nullptr;
    TalkNpcParam* mTalkNpcParam = nullptr;
    bool _208 = false;
    al::CollisionPartsConnector* mCollisionPartsConnector = nullptr;
    sead::Vector3f mScaledUpDir = sead::Vector3f::zero;
    SphinxRideAim* mSphinxRideAim = nullptr;
    s32 _230 = 0;
    s32 mAdlibCountdown = 292;
    bool mIsAdlibPlaying = false;
    u8 _239[0x200 + 7] = {};
};

static_assert(sizeof(SphinxRide) == 0x440);
