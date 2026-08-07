#pragma once

#include <basis/seadTypes.h>

namespace al {
class ComboCounter {
public:
    ComboCounter() : mCount(0) {}
    virtual void increment() { mCount++; }

    s32 getCount() const { return mCount; }
    void reset() { mCount = 0; }

private:
    s32 mCount;
};
}  // namespace al
