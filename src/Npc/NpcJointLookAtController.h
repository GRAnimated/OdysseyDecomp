#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class JointLookAtController;
class LiveActor;
}  // namespace al

class PlayerEyeSensorHitHolder;
class TalkNpcParam;

class NpcJointLookAtController {
public:
    NpcJointLookAtController(f32 lookAtDistance);

    static NpcJointLookAtController* create(al::LiveActor* actor, const TalkNpcParam* param);
    static NpcJointLookAtController* tryCreate(al::LiveActor* actor, const TalkNpcParam* param);
    static NpcJointLookAtController* tryCreate(al::LiveActor* actor, const TalkNpcParam* param,
                                               f32 lookAtDistance);

    void init(al::LiveActor* actor, const TalkNpcParam* param);
    void cancelUpdateRequest();
    void update();

    al::JointLookAtController* mJointLookAtController = nullptr;
    al::LiveActor* mActor = nullptr;
    const TalkNpcParam* mParam = nullptr;
    const char* mLastAnimName = nullptr;
    bool mIsInvalid = false;
    f32 mLookAtDistance;
    const PlayerEyeSensorHitHolder* mPlayerEyeSensorHitHolder = nullptr;
    bool mHasRequestedLookAt = false;
    sead::Vector3f mRequestedLookAtTrans = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(NpcJointLookAtController) == 0x40);
