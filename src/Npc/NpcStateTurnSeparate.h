#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class IUseEventFlowData;
class LiveActor;
}  // namespace al

class NpcStateTurnSeparate : public al::ActorStateBase {
public:
    NpcStateTurnSeparate(const char* name, al::LiveActor* actor);

    virtual void initEventUserForAction(const al::IUseEventFlowData* user);

    void startTurnToTarget(const sead::Vector3f& target);
    bool tryStartTurnToTarget(const sead::Vector3f& target, f32 threshold);
    void startTurnToDir(const sead::Vector3f& dir);
    bool tryStartTurnToDir(const sead::Vector3f& dir, f32 threshold);

private:
    u8 _20[0x30];
};

static_assert(sizeof(NpcStateTurnSeparate) == 0x50);
