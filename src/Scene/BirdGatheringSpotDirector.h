#pragma once

#include <basis/seadTypes.h>

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

private:
    u8 _padding[0x58];
};

static_assert(sizeof(BirdGatheringSpotDirector) == 0x58);
