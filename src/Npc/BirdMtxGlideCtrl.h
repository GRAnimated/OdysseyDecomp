#pragma once

#include <math/seadMatrix.h>

namespace al {
struct ActorInitInfo;
class LiveActor;
}  // namespace al

class BirdMtxGlideCtrl {
public:
    static BirdMtxGlideCtrl* tryCreateAliveWaitByLinksBird(sead::Matrix34f* birdGlideMtx,
                                                            al::LiveActor* actor,
                                                            const al::ActorInitInfo& info,
                                                            const char* linkName);
    void invalidateGlide();
    void validateGlide();
    void update();
};
