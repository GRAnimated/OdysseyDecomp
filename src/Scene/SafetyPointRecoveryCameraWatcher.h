#pragma once

namespace al {
class AreaObjGroup;
class CameraDirector;
class PlayerHolder;
}  // namespace al

class SafetyPointRecoveryCameraWatcher {
public:
    SafetyPointRecoveryCameraWatcher(al::CameraDirector*, const al::PlayerHolder*,
                                     const al::AreaObjGroup*, const al::AreaObjGroup*);

private:
    unsigned char _padding[0x88];
};

static_assert(sizeof(SafetyPointRecoveryCameraWatcher) == 0x88);
