#include "Library/Controller/KeyRepeatCtrl.h"

namespace al {

KeyRepeatCtrl::KeyRepeatCtrl() {}

void KeyRepeatCtrl::init(s32 initialMaxWait, s32 maxWait) {
    mInitialMaxWait = initialMaxWait;
    mMaxWait = maxWait;
}

void KeyRepeatCtrl::update(bool a2, bool a3) {
    if (a2 || a3) {
        if (mCounter)
            return;
    } else {
        mCounter = 0;
    }
    if (a2)
        mUpCounter++;
    else
        mUpCounter = 0;
    if (a3)
        mDownCounter++;
    else
        mDownCounter = 0;
}

void KeyRepeatCtrl::reset() {
    mUpCounter = 0;
    mDownCounter = 0;
    mCounter = true;
}

bool KeyRepeatCtrl::isUp() const {
    if (mUpCounter == 1)
        return true;
    if (mUpCounter == mInitialMaxWait)
        return true;
    return mUpCounter > mInitialMaxWait && (mUpCounter % mMaxWait);
}

bool KeyRepeatCtrl::isDown() const {
    if (mDownCounter == 1)
        return true;
    if (mDownCounter == mInitialMaxWait)
        return true;
    return mDownCounter > mInitialMaxWait && (mDownCounter - mMaxWait);
}

}  // namespace al
