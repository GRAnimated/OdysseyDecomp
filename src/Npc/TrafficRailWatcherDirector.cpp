#include "Npc/TrafficRailWatcherDirector.h"

#include "Library/LiveActor/LiveActor.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementId.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Scene/SceneObjUtil.h"

#include "Npc/TrafficRailWatcher.h"
#include "Scene/SceneObjFactory.h"

TrafficRailWatcherDirector::TrafficRailWatcherDirector() : mCount(0) {
    mEntries = new TrafficRailWatcher*[64];
    for (s32 i = 0; i < 64; i++)
        mEntries[i] = nullptr;
}

// NON_MATCHING: regswap (x19/x20, x21/x22) and stack layout (PlacementInfo/PlacementId swapped)
void TrafficRailWatcherDirector::registerActor(const al::LiveActor* actor,
                                               const al::ActorInitInfo& initInfo) {
    al::PlacementInfo placementInfo;
    al::getLinksInfo(&placementInfo, initInfo, "Rail");

    for (s32 i = 0; i < mCount; i++) {
        TrafficRailWatcher* entry = mEntries[i];
        al::PlacementId placementId;
        al::getPlacementId(&placementId, placementInfo);
        if (al::isEqualPlacementId(*entry->mPlacementId, placementId)) {
            entry->registerActor(actor);
            return;
        }
    }

    auto* watcher = new TrafficRailWatcher(placementInfo);
    mEntries[mCount] = watcher;
    mEntries[mCount]->registerActor(actor);
    mCount++;
}

TrafficRailWatcher* TrafficRailWatcherDirector::findActorRailWatcher(
    const al::LiveActor* actor) const {
    for (s32 i = 0; i < mCount; i++) {
        auto* watcher = mEntries[i];
        for (s32 j = 0; j < watcher->mActorCount; j++) {
            if (watcher->mActors[j]->actor == actor)
                return watcher;
        }
    }
    return nullptr;
}

const char* TrafficRailWatcherDirector::getSceneObjName() const {
    return "交通レール監視保持";
}

namespace rs {

void registerTrafficRailWatcher(const al::LiveActor* actor,
                                const al::ActorInitInfo& initInfo) {
    if (!al::isExistSceneObj(actor, SceneObjID_TrafficRailWatcher))
        al::setSceneObj(actor, new TrafficRailWatcherDirector(),
                        SceneObjID_TrafficRailWatcher);

    auto* director = static_cast<TrafficRailWatcherDirector*>(
        al::getSceneObj(actor, SceneObjID_TrafficRailWatcher));
    director->registerActor(actor, initInfo);
}

void stopTrafficRailByTraffic(const al::LiveActor* actor) {
    auto* director = static_cast<TrafficRailWatcherDirector*>(
        al::getSceneObj(actor, SceneObjID_TrafficRailWatcher));
    auto* watcher = director->findActorRailWatcher(actor);
    watcher->stopByTraffic(actor);
}

void restartTrafficRailByTraffic(const al::LiveActor* actor) {
    auto* director = static_cast<TrafficRailWatcherDirector*>(
        al::getSceneObj(actor, SceneObjID_TrafficRailWatcher));
    auto* watcher = director->findActorRailWatcher(actor);
    watcher->restartByTraffic(actor);
}

// NON_MATCHING: regswap in first loop, missing b skip in second loop
bool tryStopTrafficRailByOtherNpc(const al::LiveActor* actor) {
    auto* director = static_cast<TrafficRailWatcherDirector*>(
        al::getSceneObj(actor, SceneObjID_TrafficRailWatcher));
    auto* watcher = director->findActorRailWatcher(actor);

    TrafficRailActorInfo* actorInfo = nullptr;
    for (s32 i = 0; i < watcher->mActorCount; i++) {
        if (watcher->mActors[i]->actor == actor) {
            actorInfo = watcher->mActors[i];
            break;
        }
    }

    s32 j = 0;
    do {
        if (watcher->mActors[j] != actorInfo) {
            if (isNearOnTrafficRail(actor, watcher->mActors[j])) {
                actorInfo->status = 2;
                return true;
            }
        }
        j++;
    } while (j < watcher->mActorCount);
    return false;
}

// NON_MATCHING: regswap in first loop, missing b skip in second loop
bool tryRestartTrafficRailByOtherNpc(const al::LiveActor* actor) {
    auto* director = static_cast<TrafficRailWatcherDirector*>(
        al::getSceneObj(actor, SceneObjID_TrafficRailWatcher));
    auto* watcher = director->findActorRailWatcher(actor);

    TrafficRailActorInfo* actorInfo = nullptr;
    for (s32 i = 0; i < watcher->mActorCount; i++) {
        if (watcher->mActors[i]->actor == actor) {
            actorInfo = watcher->mActors[i];
            break;
        }
    }

    for (s32 i = 0; i < watcher->mActorCount; i++) {
        if (watcher->mActors[i] == actorInfo)
            continue;
        if (isNearOnTrafficRail(actor, watcher->mActors[i]))
            return false;
    }
    actorInfo->status = 0;
    return true;
}

}  // namespace rs
