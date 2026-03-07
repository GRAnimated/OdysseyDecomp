#pragma once

namespace al {
class IUseSceneObjHolder;
class LiveActor;
class PlacementInfo;
}  // namespace al

namespace rs {
al::LiveActor* tryFindCheckpointFlag(const al::IUseSceneObjHolder* holder, const char* objId);
const al::PlacementInfo* tryFindCheckpointFlagPlayerRestartInfo(
    const al::IUseSceneObjHolder* holder, const char* startId);
}  // namespace rs
