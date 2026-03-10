#pragma once

#include <container/seadRingBuffer.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
class EventFlowEventData;
class EventFlowExecutor;
class JointSpringControllerHolder;
class LiveActorGroup;
}  // namespace al

class MayorItem;
class NpcJointLookAtController;
class NpcStateReaction;
class NpcStateReactionParam;
class NpcStateWait;
class RandomWaitActionUpdater;
class SaveObjInfo;
class TalkNpcParam;

class TokimekiMayorNpc : public al::LiveActor, public al::IEventFlowEventReceiver {
public:
    TokimekiMayorNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    void control() override;

    bool receiveEvent(const al::EventFlowEventData* event) override;

    void exeWait();
    void exeReaction();

private:
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    s32 mFavorabilityRating = 0;
    al::LiveActorGroup* mMayorItemGroup = nullptr;
    s32 mLastQuestionIndex = -1;
    TalkNpcParam* mTalkNpcParam = nullptr;
    NpcJointLookAtController* mNpcJointLookAtController = nullptr;
    al::JointSpringControllerHolder* mJointSpringControllerHolder = nullptr;
    NpcStateWait* mNpcStateWait = nullptr;
    NpcStateReaction* mNpcStateReaction = nullptr;
    NpcStateReactionParam* mNpcStateReactionParam = nullptr;
    bool _160 = false;
    bool _161 = false;
    sead::RingBuffer<s32> mQuestionBuffer;
    SaveObjInfo* mSaveObjInfo = nullptr;
    SaveObjInfo* mSaveObjInfoPresent = nullptr;
    sead::Vector3f mPlayerTalkTrans = sead::Vector3f::zero;
    sead::Quatf mPlayerTalkQuat = sead::Quatf::unit;
    RandomWaitActionUpdater* mRandomWaitActionUpdater = nullptr;
};

static_assert(sizeof(TokimekiMayorNpc) == 0x1B8);
