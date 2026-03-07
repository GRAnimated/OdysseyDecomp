#pragma once

namespace al {
class Scene;
struct ActorInitInfo;
}

namespace CapManHeroDemoUtil {
void initCapManHeroDemoDirector(al::Scene* scene, const al::ActorInitInfo& initInfo);
bool isExistTalkDemoStageStart(const al::Scene*);
bool isExistTalkDemoAfterMoonRockBreakDemo(const al::Scene*);
void startTalkDemoStageStart(const al::Scene*);
void startTalkDemoAfterMoonRockBreakDemo(const al::Scene*);
}
