#include "Library/Memory/SceneHeapSetter.h"
#include "Library/System/SystemKit.h"
#include "Project/Memory/MemorySystem.h"
#include "System/ProjectInterface.h"
#include "heap/seadHeapMgr.h"

namespace al {
SceneHeapSetter::SceneHeapSetter() {
    MemorySystem* memorySystem = alProjectInterface::getSystemKit()->getMemorySystem();
    s32 t = 1;
    sead::ScopedCurrentHeapSetter setter(memorySystem->getSceneHeap());
    mLastHeap = (sead::Heap*)t;
    mCurHeap = alProjectInterface::getSystemKit()->getMemorySystem()->getSceneHeap();
}
}  // namespace al
