#pragma once

#include <container/seadPtrArray.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

class GoalMark;
class GhostPlayer;

namespace al {
class Triangle;
}

class RaceManGoal : public al::LiveActor {
public:
    RaceManGoal(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void initAfterPlacement() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;

    bool isAttachedActor(const al::LiveActor* actor) const;
    void exeWait();
    void exeEnd();
    bool isGoalPlayer() const;
    void attachActor(GhostPlayer* ghostPlayer);
    void calcMarioJointQuatPos(sead::Quatf* quat, sead::Vector3f* pos);
    const char* getRaceFirstJointName();
    s32 getAttachedGoalActorNum() const { return mGoalActors.size(); }

private:
    sead::FixedPtrArray<al::LiveActor, 5> mGoalActors;
    s32 mGoalCount = 0;
    GoalMark* mGoalMark = nullptr;
    sead::PtrArray<const char> mJointNames;
    sead::PtrArray<f32> mHeightOffsets;
    sead::Vector3f mGoalPos = sead::Vector3f::zero;
    f32 mGoalProgress = 1.0f;
    f32 mHeightOffset = 5.0f;
};

static_assert(sizeof(RaceManGoal) == 0x188);
