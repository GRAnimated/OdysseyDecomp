#pragma once

#include <math/seadQuat.h>
#include <prim/seadSafeString.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class EventFlowExecutor;
class HitSensor;
class JointSpringControllerHolder;
class MtxConnector;
class SensorMsg;
}  // namespace al

class FukankunZoomCapMessage;
class NpcJointLookAtController;
class TalkNpcActionAnimInfo;
class TalkNpcParam;
class TalkNpcStateEvent;
class Tiara;
class WorldTravelingPeachWaitState;

class WorldTravelingPeach : public al::LiveActor, public al::IEventFlowEventReceiver {
public:
    WorldTravelingPeach(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    void makeActorDead() override;
    void control() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;

    bool receiveEvent(const al::EventFlowEventData* data) override;

    void startPeachWorldHomeCastleCapDemo();
    void endPeachWorldHomeCastleCapDemo();

    void exeEvent();

    WorldTravelingPeachWaitState* mWaitState = nullptr;
    Tiara* mTiara = nullptr;
    sead::WFixedSafeString<32> mCharName;
    const TalkNpcParam* mTalkNpcParam = nullptr;
    TalkNpcActionAnimInfo* mAnimInfo = nullptr;
    TalkNpcStateEvent* mStateEvent = nullptr;
    NpcJointLookAtController* mJointLookAtController = nullptr;
    al::JointSpringControllerHolder* mSpringController = nullptr;
    FukankunZoomCapMessage* mCapMessage = nullptr;
    al::MtxConnector* mMtxConnector = nullptr;
    sead::Quatf mConnectorQuat = sead::Quatf::unit;
    bool mIsInvalidTrample = false;
};

static_assert(sizeof(WorldTravelingPeach) == 0x1C8);
