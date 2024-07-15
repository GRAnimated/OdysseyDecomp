#pragma once

#include <basis/seadTypes.h>

namespace al {
class SceneStopCtrl {
public:
    SceneStopCtrl();
    void reqeustStopScene(s32, s32);
    void update();

private:
    s32 mStopFrames = -1;
    s32 mUpdateFrames = 0;
};
}  // namespace al
