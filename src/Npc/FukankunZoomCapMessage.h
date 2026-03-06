#pragma once

namespace al {
struct ActorInitInfo;
class LiveActor;
}  // namespace al

class FukankunZoomCapMessage {
public:
    FukankunZoomCapMessage(al::LiveActor* parent);

    void init(const al::ActorInitInfo& info, const char* archiveName, const char* suffix);
    void initAfterPlacement();
    void update();

    unsigned char _0[0x50];
};
