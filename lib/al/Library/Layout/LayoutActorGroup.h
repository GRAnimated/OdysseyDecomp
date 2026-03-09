#pragma once

#include <basis/seadTypes.h>

#include "Library/HostIO/HioNode.h"

namespace al {
class LayoutActor;

class LayoutActorGroup : public HioNode {
public:
    LayoutActorGroup(const char* name, s32 maxCount);

    virtual void registerActor(LayoutActor* actor);

    LayoutActor* findDeadActor() const;
    LayoutActor* tryFindDeadActor() const;

    s32 getActorCount() const { return mActorCount; }
    LayoutActor* getActor(s32 idx) const { return mActors[idx]; }

private:
    const char* mGroupName;
    s32 mMaxActorCount;
    s32 mActorCount = 0;
    LayoutActor** mActors = nullptr;
};
}  // namespace al
