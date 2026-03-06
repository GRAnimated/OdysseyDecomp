#pragma once

#include <math/seadVector.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
class AreaObjGroup;
class EventFlowExecutor;
}  // namespace al

class SphinxRide;

class SphinxRideEvent : public al::LiveActor,
                        public al::IEventFlowEventReceiver,
                        public al::IEventFlowQueryJudge {
public:
    SphinxRideEvent(SphinxRide* sphinxRide);

    void init(const al::ActorInitInfo& info) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;

    bool receiveEvent(const al::EventFlowEventData* event) override;
    const char* judgeQuery(const char* query) const override;

    void exeCheckInArea();
    void exeEventPyramid();
    void exeEventSphinxStone();

private:
    SphinxRide* mSphinxRide = nullptr;
    sead::Vector3f mSphinxRideTrans = sead::Vector3f::zero;
    sead::Vector3f mPlayerTrans = sead::Vector3f::zero;
    sead::Vector3f mSphinxRideRotate = sead::Vector3f::zero;
    sead::Vector3f mPlayerFront = sead::Vector3f::zero;
    s32 mEventType = -1;
    al::AreaObjGroup* mAreaObjGroup = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
};

static_assert(sizeof(SphinxRideEvent) == 0x168);
