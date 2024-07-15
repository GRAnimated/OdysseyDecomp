#pragma once

#include <math/seadQuat.h>

namespace al {
class MtxConnector;
class LiveActor;
class ActorInitInfo;

MtxConnector* createMtxConnector(const al::LiveActor* actor);
MtxConnector* createMtxConnector(const al::LiveActor* actor, const sead::Quatf& quat);
MtxConnector* tryCreateMtxConnector(const LiveActor* actor, const ActorInitInfo& info);
void attachMtxConnectorToCollision(MtxConnector* mtxConnector, const LiveActor* actor, bool);
void connectPoseQT(LiveActor* actor, const MtxConnector* mtxConnector);
}  // namespace al
