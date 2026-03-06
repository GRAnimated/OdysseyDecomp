#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
class PlacementId;
class PlacementInfo;
PlacementId* createPlacementId(const PlacementInfo& placementInfo);
}  // namespace al

struct TrafficRailActorInfo {
    const al::LiveActor* actor;
    s32 status;
};

class TrafficRailWatcher {
public:
    TrafficRailWatcher(const al::PlacementInfo& placementInfo)
        : mPlacementId(nullptr), mActorCount(0), mActors(nullptr) {
        mPlacementId = al::createPlacementId(placementInfo);
        mActors = new TrafficRailActorInfo*[32];
        for (s32 i = 0; i < 32; i++)
            mActors[i] = nullptr;
    }

    void registerActor(const al::LiveActor* actor) {
        auto* info = new TrafficRailActorInfo;
        info->actor = actor;
        info->status = 0;
        mActors[mActorCount] = info;
        mActorCount++;
    }

    void stopByTraffic(const al::LiveActor* actor) {
        TrafficRailActorInfo** p = mActors;
        TrafficRailActorInfo* info;
        do {
            info = *p++;
        } while (info->actor != actor);
        info->status = 1;
    }

    void restartByTraffic(const al::LiveActor* actor) {
        TrafficRailActorInfo** p = mActors;
        TrafficRailActorInfo* info;
        do {
            info = *p++;
        } while (info->actor != actor);
        info->status = 0;
    }

    bool isEqual(const al::PlacementInfo& placementInfo) const;
    bool isExist(const al::LiveActor* actor) const;
    bool tryStopByOtherNpc(const al::LiveActor* actor);
    bool tryRestartByOtherNpc(const al::LiveActor* actor);

    al::PlacementId* mPlacementId;
    s32 mActorCount;
    TrafficRailActorInfo** mActors;
};

static_assert(sizeof(TrafficRailWatcher) == 0x18);

bool isNearOnTrafficRail(const al::LiveActor* actor, const TrafficRailActorInfo* otherEntry);
