#include "Library/Layout/LayoutActorGroup.h"

#include "Library/Layout/LayoutActor.h"

namespace al {

LayoutActorGroup::LayoutActorGroup(const char* name, s32 maxCount)
    : mGroupName(name), mMaxActorCount(maxCount) {
    mActors = new LayoutActor*[maxCount];
}

void LayoutActorGroup::registerActor(LayoutActor* actor) {
    mActors[mActorCount] = actor;
    mActorCount++;
}

LayoutActor* LayoutActorGroup::findDeadActor() const {
    for (s32 i = 0; i < mActorCount; i++)
        if (!mActors[i]->isAlive())
            return mActors[i];
    return mActors[0];
}

LayoutActor* LayoutActorGroup::tryFindDeadActor() const {
    for (s32 i = 0; i < mActorCount; i++)
        if (!mActors[i]->isAlive())
            return mActors[i];
    return nullptr;
}

}  // namespace al
