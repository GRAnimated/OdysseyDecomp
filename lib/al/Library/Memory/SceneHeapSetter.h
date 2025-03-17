#pragma once

#include <heap/seadHeap.h>

namespace al {
class SceneHeapSetter {
public:
    SceneHeapSetter();

    sead::Heap* getLastHeap() const { return mLastHeap; }

    sead::Heap* getCurHeap() const { return mCurHeap; }

private:
    sead::Heap* mLastHeap;
    sead::Heap* mCurHeap;
};
}  // namespace al
