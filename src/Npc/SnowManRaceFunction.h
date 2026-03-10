#pragma once

namespace al {
class LiveActor;
}

namespace SnowManRaceFunction {
void registerNpcToRaceWatcher(al::LiveActor* actor);
f32 calcRaceProgress(const al::LiveActor* actor);
f32 calcRaceProgressPlayer(const al::LiveActor* actor);
s32 getLapNum(const al::LiveActor* actor);
}  // namespace SnowManRaceFunction
