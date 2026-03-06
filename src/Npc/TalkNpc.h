#pragma once

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class AreaObjGroup;
class EventFlowEventData;
class EventFlowExecutor;
class HitSensor;
class JointSpringControllerHolder;
class MtxConnector;
class SensorMsg;
}  // namespace al

class BgmAnimeSynchronizer;
class BirdMtxGlideCtrl;
class NpcJointLookAtController;
class PlayerEyeSensorHitHolder;
class RandomActionUpdater;
class TalkNpcActionAnimInfo;
class TalkNpcCap;
class TalkNpcParam;
class TalkNpcPartialAnimCtrl;
class TalkNpcStateEvent;
class TalkNpcSubActorSyncHolder;
class TalkNpcSwitchActionPlayerHolder;

class TalkNpc : public al::LiveActor, public al::IEventFlowEventReceiver {
public:
    TalkNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    void movement() override;
    void calcAnim() override;
    void updateCollider() override;
    void kill() override;
    void endClipped() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;

    bool receiveEvent(const al::EventFlowEventData* data) override;

    void onFukankunCameraNearClipSetting();
    void offFukankunCameraNearClipSetting();
    TalkNpcStateEvent* tryGetCurrentEventState() const;
    al::EventFlowExecutor* tryGetCurrentEvent() const;
    void startDemoKoopa(const sead::Vector3f& pos);
    void endDemoKoopa();

    void exeEvent();
    void exeHackEvent();
    void exeKinopioBrigadeFirstEvent();
    void exeDemoKoopa();

    TalkNpcCap* mTalkNpcCap = nullptr;
    TalkNpcSubActorSyncHolder* mSubActorSyncHolder = nullptr;
    al::LiveActor* mHideAndSeekCapMan = nullptr;
    const TalkNpcParam* mTalkNpcParam = nullptr;
    TalkNpcStateEvent* mStateEvent = nullptr;
    TalkNpcStateEvent* mStateEventHacking = nullptr;
    TalkNpcStateEvent* mStateEventFirst = nullptr;
    PlayerEyeSensorHitHolder* mPlayerEyeSensorHitHolder = nullptr;
    s32 mMovementType = 0;
    s32 mHackingEventHackType = -1;
    NpcJointLookAtController* mJointLookAtController = nullptr;
    void* mJointGroundSmoothController = nullptr;
    al::JointSpringControllerHolder* mJointSpringControllerHolder = nullptr;
    void* mHipJointController = nullptr;
    void* mSpineJointController = nullptr;
    TalkNpcActionAnimInfo* mActionAnimInfo = nullptr;
    TalkNpcPartialAnimCtrl* mPartialAnimCtrl = nullptr;
    RandomActionUpdater* mRandomActionUpdater = nullptr;
    TalkNpcSwitchActionPlayerHolder* mSwitchActionPlayerHolder = nullptr;
    void* mClippingInfo = nullptr;
    void* mVariableFrameRateAnimator = nullptr;
    al::MtxConnector* mMtxConnector = nullptr;
    sead::Quatf mConnectorQuat = sead::Quatf::unit;
    sead::Vector3f mInitTrans = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mInitFront = {0.0f, 0.0f, 0.0f};
    BirdMtxGlideCtrl* mBirdMtxGlideCtrl = nullptr;
    sead::Matrix34f mBirdGlideMtx = sead::Matrix34f::ident;
    BgmAnimeSynchronizer* mBgmAnimeSynchronizer = nullptr;
    al::AreaObjGroup* mNpcForceMaterialCodeAreaGroup = nullptr;
    void* mLinkAreaGroup = nullptr;
    s32 mWaterCheckCount = 0;
    bool mIsInvalidTrample = false;
    bool mIsEyeSensorInvalid = false;
    bool _236 = false;
};

static_assert(sizeof(TalkNpc) == 0x238);
