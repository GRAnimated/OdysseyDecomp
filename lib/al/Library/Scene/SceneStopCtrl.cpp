#include "Library/Scene/SceneStopCtrl.h"

namespace al {
SceneStopCtrl::SceneStopCtrl() {}

void SceneStopCtrl::reqeustStopScene(s32 a1, s32 a2) {
    if (mStopFrames <= 0 && mUpdateFrames <= 0)
        mUpdateFrames = a2;
    if (mStopFrames < a1)
        mStopFrames = a1;
}

void SceneStopCtrl::update() {
    if (mUpdateFrames >= 1)
        mUpdateFrames--;
    else if (mStopFrames >= 1)
        mStopFrames--;
}
}  // namespace al
