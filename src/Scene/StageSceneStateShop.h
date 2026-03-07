#pragma once

#include "Library/LiveActor/LiveActorGroup.h"
#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
}

class ShoppingWatcher;
class StageScene;
class StageSceneLayout;

class StageSceneStateShop : public al::HostStateBase<StageScene> {
public:
    StageSceneStateShop(const char*, StageScene*, StageSceneLayout*);
    void initAndSetting(al::DeriveActorGroup<ShoppingWatcher>*, const al::ActorInitInfo&);
};
