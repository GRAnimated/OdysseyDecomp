#pragma once

namespace al {
class AreaObjGroup;
class CameraDirector;
class ClippingJudge;
class SceneObjHolder;
struct ActorInitInfo;
}  // namespace al

class BirdGatheringSpotDirector {
public:
    BirdGatheringSpotDirector();
    void init(const al::ActorInitInfo&, const al::AreaObjGroup*, al::CameraDirector*,
              al::SceneObjHolder*, const al::ClippingJudge*, const char*);
};
