#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
class EventFlowExecutor;
class JointSpringControllerHolder;
class MtxConnector;
}  // namespace al

class NpcEventStateScare;
class NpcJointLookAtController;
class NpcStateReaction;
class RandomActionUpdater;
class TalkNpcActionAnimInfo;
class TalkNpcParam;

struct NpcFacialAnimCache {
    al::LiveActor* actor;
    TalkNpcParam* talkNpcParam;
    const char* cachedAnimName;
    bool isEnabled;
    bool cachedIsValid;
};

class TimeBalloonNpc : public al::LiveActor {
public:
    TimeBalloonNpc(const char* name) : al::LiveActor(name) {}

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) override;
    void control() override;

    void exeEvent();
    void exeReaction();
    bool isReaction() const;
    void exeScared();

private:
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    s32 mScenarioNo = 0;
    u8 _114[0xC] = {};
    sead::Vector3f mTutorialBalloonTrans = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mPlayerRestartTrans = {0.0f, 0.0f, 0.0f};
    al::MtxConnector* mMtxConnector = nullptr;
    al::LiveActor* mBalloonActor = nullptr;
    al::JointSpringControllerHolder* mJointSpringControllerHolder = nullptr;
    NpcStateReaction* mNpcStateReaction = nullptr;
    TalkNpcParam* mTalkNpcParam = nullptr;
    NpcJointLookAtController* mNpcJointLookAtController = nullptr;
    u8 _168[0x10] = {};
    NpcFacialAnimCache* mFacialAnimCache = nullptr;
    NpcEventStateScare* mNpcEventStateScare = nullptr;
    TalkNpcActionAnimInfo* mTalkNpcActionAnimInfo = nullptr;
    RandomActionUpdater* mRandomActionUpdater = nullptr;
};

static_assert(sizeof(TimeBalloonNpc) == 0x198);
