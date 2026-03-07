#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
class WipeSimple;
}  // namespace al

class CinemaCaption;
class EventFlowSceneExecuteCtrl;
class GameDataHolder;
class PlayGuideSkip;
class StageSceneLayout;
class StageSceneStateCollectionList;
class StageSceneStateGetShine;
class StageSceneStateGetShineMain;
class StageSceneStateMiniGameRanking;
class StageSceneStateSkipDemo;
class StageSceneStateWorldMap;

class StageSceneStateTalk : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateTalk(const char*, al::Scene*, EventFlowSceneExecuteCtrl*, StageSceneLayout*,
                        CinemaCaption*, PlayGuideSkip*, StageSceneStateMiniGameRanking*,
                        StageSceneStateSkipDemo*, StageSceneStateWorldMap*,
                        StageSceneStateGetShine*, StageSceneStateGetShineMain*,
                        StageSceneStateCollectionList*, GameDataHolder*, al::WipeSimple*);

private:
    u8 _padding[0x70];
};

static_assert(sizeof(StageSceneStateTalk) == 0x90);
