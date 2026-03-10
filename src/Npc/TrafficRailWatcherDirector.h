#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.h"

class TrafficRailWatcher;

namespace al {
class LiveActor;
struct ActorInitInfo;
}  // namespace al

class TrafficRailWatcherDirector : public al::ISceneObj {
public:
    TrafficRailWatcherDirector();

    void registerActor(const al::LiveActor* actor, const al::ActorInitInfo& initInfo);
    TrafficRailWatcher* findActorRailWatcher(const al::LiveActor* actor) const;

    const char* getSceneObjName() const override;
    ~TrafficRailWatcherDirector() override = default;

    s32 mCount;
    TrafficRailWatcher** mEntries;
};

static_assert(sizeof(TrafficRailWatcherDirector) == 0x18);
