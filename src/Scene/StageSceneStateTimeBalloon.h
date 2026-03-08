#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
}

class GameDataHolder;
class MiniGameMenu;
class StageScene;
class StageSceneLayout;
class StageSceneStateWarp;
class TimeBalloonDirector;
class TimeBalloonNpc;
class TimeBalloonSequenceInfo;

class StageSceneStateTimeBalloon : public al::HostStateBase<StageScene> {
public:
    StageSceneStateTimeBalloon(StageScene*, TimeBalloonDirector*, TimeBalloonSequenceInfo*,
                               GameDataHolder*, StageSceneLayout*, const al::ActorInitInfo&,
                               TimeBalloonNpc*, bool, MiniGameMenu*, void*,
                               StageSceneStateWarp*);
    virtual bool isAutoStart() const;

private:
    u8 _padding[0x138];
};

static_assert(sizeof(StageSceneStateTimeBalloon) == 0x158);
