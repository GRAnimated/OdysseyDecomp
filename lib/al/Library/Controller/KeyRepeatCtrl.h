#pragma once

#include <basis/seadTypes.h>

namespace al {
class KeyRepeatCtrl {
public:
    KeyRepeatCtrl();

    void init(s32, s32);
    void update(bool, bool);
    void reset();
    bool isUp() const;
    bool isDown() const;

private:
    s32 mInitialMaxWait = 0;
    s32 mMaxWait = 0;
    s32 mUpCounter = 0;
    s32 mDownCounter = 0;
    bool mCounter = true;
};
}  // namespace al
