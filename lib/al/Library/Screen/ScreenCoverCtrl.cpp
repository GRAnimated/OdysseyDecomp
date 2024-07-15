#include "Library/Screen/ScreenCoverCtrl.h"

namespace al {
ScreenCoverCtrl::ScreenCoverCtrl() {}

void ScreenCoverCtrl::requestCaptureScreenCover(s32 duration) {
    if (mCoverFrames < duration) {
        if (mCoverFrames <= 0)
            mIsActive = true;
        mCoverFrames = duration;
    }
}

void ScreenCoverCtrl::update() {
    if (mCoverFrames >= 1) {
        mCoverFrames--;
        if (mCoverFrames == 0)
            mIsActive = false;
    }
}
}  // namespace al
